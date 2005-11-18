/*
 * ====================================================================
 * Copyright (c) 2005 CollabNet.  All rights reserved.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at http://subversion.tigris.org/license-1.html.
 * If newer versions of this license are posted there, you may use a
 * newer version instead, at your option.
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://subversion.tigris.org/.
 * ====================================================================
 */

#include "svn_cmdline.h"
#include "svn_config.h"
#include "svn_client.h"
#include "svn_pools.h"
#include "svn_delta.h"
#include "svn_path.h"
#include "svn_auth.h"
#include "svn_opt.h"
#include "svn_ra.h"

#include "svn_private_config.h"

#include <apr_network_io.h>
#include <apr_uuid.h>

#define PROP_PREFIX            "svn:sync-"

#define LOCK_PROP              PROP_PREFIX "lock"
#define FROM_URL_PROP          PROP_PREFIX "from-url"
#define FROM_UUID_PROP         PROP_PREFIX "from-uuid"
#define LAST_MERGED_REV_PROP   PROP_PREFIX "last-merged-rev"
#define CURRENTLY_COPYING_PROP PROP_PREFIX "currently-copying"

static svn_opt_subcommand_t initialize, synchronize, help;

enum {
  svnsync_opt_source_url       = SVN_OPT_FIRST_LONGOPT_ID,
  svnsync_opt_non_interactive,
  svnsync_opt_no_auth_cache,
  svnsync_opt_auth_username,
  svnsync_opt_auth_password,
  svnsync_opt_config_dir,
  svnsync_opt_version,
  svnsync_opt_help
};

#define SVNSYNC_OPTS_DEFAULT svnsync_opt_non_interactive, \
                             svnsync_opt_no_auth_cache, \
                             svnsync_opt_auth_username, \
                             svnsync_opt_auth_password, \
                             svnsync_opt_config_dir

static const svn_opt_subcommand_desc_t svnsync_cmd_table[] =
  {
    { "initialize", initialize, { "init" },
      N_("usage: svnsync initialize DEST_URL --source-url SOURCE_URL\n"
         "Initialize a destination repository."),
      { SVNSYNC_OPTS_DEFAULT,
        svnsync_opt_source_url } },
    { "synchronize", synchronize, { "sync" },
      N_("usage: svnsync synchronize DEST_URL\n"
         "Transfer all pending revisions from source to destination.\n"),
      { SVNSYNC_OPTS_DEFAULT } },
    { "help", help, { 0 },
      N_("usage: svnsync help [SUBCOMMAND...]\n"
         "Describe the usage of this program or its subcommands.\n"),
      { svnsync_opt_version } },
    { NULL, NULL, { 0 }, NULL, { 0 } } 
  };

static const apr_getopt_option_t svnsync_options[] =
  {
    {"source-url",     svnsync_opt_source_url, 1,
                       N_("The url to synchronize from")},
    {"non-interactive", svnsync_opt_non_interactive, 0,
                       N_("do no interactive prompting") },
    {"no-auth-cache",  svnsync_opt_no_auth_cache, 0,
                       N_("do not cache authentication tokens") },
    {"username",       svnsync_opt_auth_username, 1,
                       N_("specify a username ARG") },
    {"password",       svnsync_opt_auth_password, 1,
                       N_("specify a password ARG") },
    {"config-dir",     svnsync_opt_config_dir, 1,
                       N_("read user configuration files from directory ARG")},
    {"version",        svnsync_opt_version, 0,
                       N_("show version information")},
    {"help",           svnsync_opt_help, 0,
                       N_("show help on a subcommand")},
    {NULL,             svnsync_opt_help, 0,
                       N_("show help on a subcommand")},
    { 0, 0, 0, 0 }
  };

typedef struct {
  const char *source_url;
  svn_auth_baton_t *auth_baton;
  svn_boolean_t non_interactive;
  svn_boolean_t no_auth_cache;
  const char *auth_username;
  const char *auth_password;
  const char *config_dir;
  apr_hash_t *config;
  svn_boolean_t version;
  svn_boolean_t help;
} opt_baton_t;

static volatile sig_atomic_t cancelled = FALSE;

static void
signal_handler (int signum)
{
  apr_signal (signum, SIG_IGN);
  cancelled = TRUE;
}

static svn_error_t *
check_cancel (void *baton)
{
  if (cancelled)
    return svn_error_create (SVN_ERR_CANCELLED, NULL, _("Caught signal"));
  else
    return SVN_NO_ERROR;
}

static svn_error_t *
check_lib_versions (void)
{
  static const svn_version_checklist_t checklist[] =
    {
      { "svn_subr",  svn_subr_version },
      { "svn_delta", svn_delta_version },
      { "svn_ra",    svn_ra_version },
      { NULL, NULL }
    };

  SVN_VERSION_DEFINE (my_version);

  return svn_ver_check_list (&my_version, checklist);
}

/* XXX need a way to break the lock. */
static svn_error_t *
get_lock (svn_ra_session_t *session, apr_pool_t *pool)
{
  char uuid_str[APR_UUID_FORMATTED_LENGTH + 1] = { 0 };
  char hostname_str[APRMAXHOSTLEN + 1] = { 0 };
  svn_string_t *mylocktoken, *reposlocktoken;
  apr_status_t apr_err;
  apr_pool_t *subpool;
  apr_uuid_t uuid;

  apr_err = apr_gethostname (hostname_str, sizeof (hostname_str), pool);
  if (apr_err)
    return svn_error_wrap_apr (apr_err, "Can't get local hostname");

  apr_uuid_get (&uuid);

  apr_uuid_format (uuid_str, &uuid);

  mylocktoken = svn_string_createf (pool, "%s:%s", hostname_str, uuid_str);

  subpool = svn_pool_create (pool);

  for (;;)
    {
      svn_pool_clear (subpool);

      SVN_ERR (svn_ra_rev_prop (session, 0, LOCK_PROP, &reposlocktoken,
                                subpool));

      if (reposlocktoken)
        {
          /* did we get it?   if so, we're done, otherwise we sleep. */

          if (strcmp (reposlocktoken->data, mylocktoken->data) == 0)
            break;
          else
            apr_sleep (1); /* XXX notify user */
        }
      else
        {
          SVN_ERR (svn_ra_change_rev_prop (session, 0, LOCK_PROP, mylocktoken,
                                           subpool));
        }
    }

  return SVN_NO_ERROR;
}

typedef svn_error_t *(*with_locked_func_t) (svn_ra_session_t *session,
                                            void *baton,
                                            apr_pool_t *pool);

/* Lock the destination repository, then execute the given function/baton
 * pair while holding the lock.  Finally, drop the lock once it finishes. */
static svn_error_t *
with_locked (svn_ra_session_t *session,
             with_locked_func_t func,
             void *baton,
             apr_pool_t *pool)
{
  svn_error_t *err, *err2;

  SVN_ERR (get_lock (session, pool));

  err = func (session, baton, pool);

  err2 = svn_ra_change_rev_prop (session, 0, LOCK_PROP, NULL, pool);
  if (err2 && err)
    {
      svn_error_clear (err2); /* XXX what to do here? */

      return err;
    }
  else if (err2)
    {
      return err2;
    }
  else
    {
      return err;
    }
}

static svn_error_t *
open_tmp_file (apr_file_t **fp, void *callback_baton, apr_pool_t *pool)
{
  const char *path;

  SVN_ERR (svn_io_temp_dir (&path, pool));

  path = svn_path_join (path, "tempfile", pool);

  SVN_ERR (svn_io_open_unique_file (fp, NULL, path, ".tmp", TRUE, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
check_if_session_is_at_repos_root (svn_ra_session_t *sess,
                                   const char *url,
                                   apr_pool_t *pool)
{
  const char *sess_root;

  SVN_ERR (svn_ra_get_repos_root (sess, &sess_root, pool));

  if (strcmp (url, sess_root) == 0)
    return SVN_NO_ERROR;
  else
    return svn_error_createf
      (APR_EINVAL, NULL,
       _("Session is rooted at '%s' but the repos root is '%s'"),
       url, sess_root);
}

typedef struct {
  const char *from_url;
  const char *to_url;
  apr_hash_t *config;
  svn_ra_callbacks2_t *callbacks;
} init_baton_t;

static svn_error_t *
do_initialize (svn_ra_session_t *to_session, void *b, apr_pool_t *pool)
{
  svn_ra_session_t *from_session;
  init_baton_t *baton = b;
  svn_string_t *from_url;
  apr_hash_t *revprops;
  apr_hash_index_t *hi;
  svn_revnum_t latest;
  const char *uuid;

  /* First, sanity check to see that we're copying into a brand new repos. */

  SVN_ERR (svn_ra_get_latest_revnum (to_session, &latest, pool));

  if (latest != 0)
    return svn_error_create
      (APR_EINVAL, NULL, "Cannot initialize a repository with content in it");

  /* And check to see if anyone's run initialize on it before...
   * We may want a --force option to override this check. */

  SVN_ERR (svn_ra_rev_prop (to_session, 0, FROM_URL_PROP, &from_url, pool));

  if (from_url)
    return svn_error_createf
      (APR_EINVAL, NULL,
       "Destination repository is already synchronizing from %s",
       from_url->data);

  /* Now fill in our bookkeeping info in the dest repository. */

  SVN_ERR (svn_ra_open2 (&from_session, baton->from_url, baton->callbacks,
                         baton, baton->config, pool));

  SVN_ERR (check_if_session_is_at_repos_root (from_session, baton->from_url,
                                              pool));

  SVN_ERR (svn_ra_change_rev_prop (to_session, 0, FROM_URL_PROP,
                                   svn_string_create (baton->from_url, pool),
                                   pool));

  SVN_ERR (svn_ra_get_uuid (from_session, &uuid, pool));

  SVN_ERR (svn_ra_change_rev_prop (to_session, 0, FROM_UUID_PROP,
                                   svn_string_create (uuid, pool), pool));

  SVN_ERR (svn_ra_change_rev_prop (to_session, 0, LAST_MERGED_REV_PROP,
                                   svn_string_create ("0", pool), pool));

  /* Finally, copy all non-svnsync revprops from rev 0 of the source repos
   * into the dest repos. */

  SVN_ERR (svn_ra_rev_proplist (from_session, 0, &revprops, pool));

  for (hi = apr_hash_first (pool, revprops);
       hi;
       hi = apr_hash_next (hi))
    {
      const char *pname;
      svn_string_t *pval;
      const void *key;
      void *val;

      apr_hash_this (hi, &key, NULL, &val);

      pname = key;
      pval = val;

      if (strncmp (pname, PROP_PREFIX, sizeof (PROP_PREFIX) - 1) != 0)
        SVN_ERR (svn_ra_change_rev_prop (to_session, 0, pname, pval, pool));
    }

  /* It would be nice if we could set the dest repos UUID to be equal to the
   * UUID of the source repos, at least optionally.  That way people could
   * check out/log/diff using a local fast mirror, but switch --relocate to
   * the actual final repository in order to make changes...
   *
   * I don't think the RA layer has a way to set a UUID though, so we may
   * be stuck using a svnadmin call to do that for us. */

  return SVN_NO_ERROR;
}

static svn_error_t *
initialize (apr_getopt_t *os, void *b, apr_pool_t *pool)
{
  svn_ra_callbacks2_t callbacks = { 0 };
  svn_ra_session_t *to_session;
  opt_baton_t *opt_baton = b;
  apr_array_header_t *args;
  init_baton_t baton;

  SVN_ERR (svn_opt_parse_num_args (&args, os, 1, pool));

  baton.to_url = APR_ARRAY_IDX (args, 0, const char *);
  baton.from_url = opt_baton->source_url;

  baton.config = opt_baton->config;

  callbacks.open_tmp_file = open_tmp_file;
  callbacks.auth_baton = opt_baton->auth_baton;

  baton.callbacks = &callbacks;

  SVN_ERR (svn_ra_open2 (&to_session,
                         baton.to_url,
                         &callbacks,
                         &baton,
                         baton.config,
                         pool));

  SVN_ERR (check_if_session_is_at_repos_root (to_session, baton.to_url, pool));

  SVN_ERR (with_locked (to_session, do_initialize, &baton, pool));

  return SVN_NO_ERROR;
}

typedef struct {
  apr_hash_t *config;
  svn_ra_callbacks2_t *callbacks;
  const char *to_url;
} sync_baton_t;

static svn_error_t *
commit_callback (const svn_commit_info_t *commit_info,
                 void *baton,
                 apr_pool_t *pool)
{
  SVN_ERR (svn_cmdline_printf (pool, "Committing rev %ld\n",
                               commit_info->revision));

  return SVN_NO_ERROR;
}

/* This editor has a couple of jobs.
 *
 * First, it needs to filter out the propchanges that can't be passed over
 * libsvn_ra.
 *
 * Second, it needs to adjust for the fact that we might not actually have
 * permission to see all of the data from the remote repository, which means
 * we could get revisions that are totally empty from our point of view.
 *
 * Third, it needs to adjust copyfrom paths, adding the root url for the
 * destination repository to the beginning of them.
 */

typedef struct {
  const svn_delta_editor_t *wrapped_editor;
  void *wrapped_edit_baton;

  /* The URL we're copying into, so we can set up copyfrom urls correctly. */
  const char *to_url;

  svn_boolean_t called_open_root;

  svn_revnum_t base_revision;
} edit_baton_t;

typedef struct {
  void *edit_baton;
  void *wrapped_dir_baton;
} dir_baton_t;

typedef struct {
  void *edit_baton;
  void *wrapped_file_baton;
} file_baton_t;

static svn_error_t *
set_target_revision (void *edit_baton,
                     svn_revnum_t target_revision,
                     apr_pool_t *pool)
{
  edit_baton_t *eb = edit_baton;

  SVN_ERR (eb->wrapped_editor->set_target_revision (eb->wrapped_edit_baton,
                                                    target_revision, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
open_root (void *edit_baton,
           svn_revnum_t base_revision,
           apr_pool_t *pool,
           void **root_baton)
{
  edit_baton_t *eb = edit_baton;
  dir_baton_t *dir_baton = apr_palloc (pool, sizeof (*dir_baton));

  SVN_ERR (eb->wrapped_editor->open_root (eb->wrapped_edit_baton,
                                          base_revision, pool,
                                          &dir_baton->wrapped_dir_baton));

  eb->called_open_root = TRUE;

  dir_baton->edit_baton = edit_baton;

  *root_baton = dir_baton;

  return SVN_NO_ERROR;
}

static svn_error_t *
delete_entry (const char *path,
              svn_revnum_t base_revision,
              void *parent_baton,
              apr_pool_t *pool)
{
  dir_baton_t *pb = parent_baton;
  edit_baton_t *eb = pb->edit_baton;

  SVN_ERR (eb->wrapped_editor->delete_entry (path, base_revision,
                                             pb->wrapped_dir_baton, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
add_directory (const char *path,
               void *parent_baton,
               const char *copyfrom_path,
               svn_revnum_t copyfrom_rev,
               apr_pool_t *pool,
               void **child_baton)
{
  dir_baton_t *pb = parent_baton;
  edit_baton_t *eb = pb->edit_baton;
  dir_baton_t *b = apr_palloc (pool, sizeof (*b));

  if (copyfrom_path)
    {
      copyfrom_path = apr_psprintf (pool, "%s%s", eb->to_url,
                                    copyfrom_path);
    }

  SVN_ERR (eb->wrapped_editor->add_directory (path, pb->wrapped_dir_baton,
                                              copyfrom_path,
                                              copyfrom_rev, pool,
                                              &b->wrapped_dir_baton));

  b->edit_baton = eb;
  *child_baton = b;

  return SVN_NO_ERROR;
}

static svn_error_t *
open_directory (const char *path,
                void *parent_baton,
                svn_revnum_t base_revision,
                apr_pool_t *pool,
                void **child_baton)
{
  dir_baton_t *pb = parent_baton;
  edit_baton_t *eb = pb->edit_baton;
  dir_baton_t *db = apr_palloc (pool, sizeof (*db));

  SVN_ERR (eb->wrapped_editor->open_directory (path, pb->wrapped_dir_baton,
                                               base_revision, pool,
                                               &db->wrapped_dir_baton));

  db->edit_baton = eb;
  *child_baton = db;

  return SVN_NO_ERROR;
}

static svn_error_t *
add_file (const char *path,
          void *parent_baton,
          const char *copyfrom_path,
          svn_revnum_t copyfrom_rev,
          apr_pool_t *pool,
          void **file_baton)
{
  dir_baton_t *pb = parent_baton;
  edit_baton_t *eb = pb->edit_baton;
  file_baton_t *fb = apr_palloc (pool, sizeof (*fb));

  if (copyfrom_path)
    {
      copyfrom_path = apr_psprintf (pool, "%s%s", eb->to_url,
                                    copyfrom_path);
    }

  SVN_ERR (eb->wrapped_editor->add_file (path, pb->wrapped_dir_baton,
                                         copyfrom_path, copyfrom_rev,
                                         pool, &fb->wrapped_file_baton));

  fb->edit_baton = eb;

  *file_baton = fb;

  return SVN_NO_ERROR;
}

static svn_error_t *
open_file (const char *path,
           void *parent_baton,
           svn_revnum_t base_revision,
           apr_pool_t *pool,
           void **file_baton)
{
  dir_baton_t *pb = parent_baton;
  edit_baton_t *eb = pb->edit_baton;
  file_baton_t *fb = apr_palloc (pool, sizeof (*fb));

  SVN_ERR (eb->wrapped_editor->open_file (path, pb->wrapped_dir_baton,
                                          base_revision, pool,
                                          &fb->wrapped_file_baton));

  fb->edit_baton = eb;

  *file_baton = fb;

  return SVN_NO_ERROR;
}

static svn_error_t *
apply_textdelta (void *file_baton,
                 const char *base_checksum,
                 apr_pool_t *pool,
                 svn_txdelta_window_handler_t *handler,
                 void **handler_baton)
{
  file_baton_t *fb = file_baton;
  edit_baton_t *eb = fb->edit_baton;

  SVN_ERR (eb->wrapped_editor->apply_textdelta (fb->wrapped_file_baton,
                                                base_checksum, pool,
                                                handler, handler_baton));
  return SVN_NO_ERROR;
}

static svn_error_t *
close_file (void *file_baton,
            const char *text_checksum,
            apr_pool_t *pool)
{
  file_baton_t *fb = file_baton;
  edit_baton_t *eb = fb->edit_baton;

  SVN_ERR (eb->wrapped_editor->close_file (fb->wrapped_file_baton,
                                           text_checksum, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
absent_file (const char *path,
             void *file_baton,
             apr_pool_t *pool)
{
  file_baton_t *fb = file_baton;
  edit_baton_t *eb = fb->edit_baton;

  SVN_ERR (eb->wrapped_editor->absent_file (path, fb->wrapped_file_baton,
                                            pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
close_directory (void *dir_baton,
                 apr_pool_t *pool)
{
  dir_baton_t *db = dir_baton;
  edit_baton_t *eb = db->edit_baton;

  SVN_ERR (eb->wrapped_editor->close_directory (db->wrapped_dir_baton,
                                                pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
absent_directory (const char *path,
                  void *dir_baton,
                  apr_pool_t *pool)
{
  dir_baton_t *db = dir_baton;
  edit_baton_t *eb = db->edit_baton;

  SVN_ERR (eb->wrapped_editor->absent_directory (path, db->wrapped_dir_baton,
                                                 pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
change_file_prop (void *file_baton,
                  const char *name,
                  const svn_string_t *value,
                  apr_pool_t *pool)
{
  file_baton_t *fb = file_baton;
  edit_baton_t *eb = fb->edit_baton;

  /* only regular properties can pass over libsvn_ra */
  if (svn_property_kind (NULL, name) != svn_prop_regular_kind)
    return SVN_NO_ERROR;

  SVN_ERR (eb->wrapped_editor->change_file_prop (fb->wrapped_file_baton,
                                                 name, value, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
change_dir_prop (void *dir_baton,
                 const char *name,
                 const svn_string_t *value,
                 apr_pool_t *pool)
{
  dir_baton_t *db = dir_baton;
  edit_baton_t *eb = db->edit_baton;

  /* only regular properties can pass over libsvn_ra */
  if (svn_property_kind (NULL, name) != svn_prop_regular_kind)
    return SVN_NO_ERROR;

  SVN_ERR (eb->wrapped_editor->change_dir_prop (db->wrapped_dir_baton,
                                                name, value, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
close_edit (void *edit_baton,
            apr_pool_t *pool)
{
  edit_baton_t *eb = edit_baton;

  /* If we haven't opened the root yet, that means we're transfering an
   * empty revision, probably because we aren't allowed to see the contents
   * for some reason.  In any event, we need to open the root before we can
   * close it, or the commit will fail. */

  if (! eb->called_open_root)
    {
      void *baton;

      SVN_ERR (eb->wrapped_editor->open_root (eb->wrapped_edit_baton,
                                              eb->base_revision, pool,
                                              &baton));
    }

  SVN_ERR (eb->wrapped_editor->close_edit (eb->wrapped_edit_baton, pool));

  return SVN_NO_ERROR;
}

/* Create a wrapper editor that holds our commit editor.  The only
 * responsibility of this wrapper is to make sure that we don't try to
 * pass "weird" properties over the wire that libsvn_ra complains about.
 */
static svn_error_t *
get_sync_editor (const svn_delta_editor_t *wrapped_editor,
                 void *wrapped_edit_baton,
                 svn_revnum_t base_revision,
                 const char *to_url,
                 const svn_delta_editor_t **editor,
                 void **edit_baton,
                 apr_pool_t *pool)
{
  svn_delta_editor_t *tree_editor = svn_delta_default_editor (pool);
  edit_baton_t *eb = apr_palloc (pool, sizeof (*eb));

  tree_editor->set_target_revision = set_target_revision;
  tree_editor->open_root = open_root;
  tree_editor->delete_entry = delete_entry;
  tree_editor->add_directory = add_directory;
  tree_editor->open_directory = open_directory;
  tree_editor->change_dir_prop = change_dir_prop;
  tree_editor->close_directory = close_directory;
  tree_editor->absent_directory = absent_directory;
  tree_editor->add_file = add_file;
  tree_editor->open_file = open_file;
  tree_editor->apply_textdelta = apply_textdelta;
  tree_editor->change_file_prop = change_file_prop;
  tree_editor->close_file = close_file;
  tree_editor->absent_file = absent_file;
  tree_editor->close_edit = close_edit;

  eb->wrapped_editor = wrapped_editor;
  eb->wrapped_edit_baton = wrapped_edit_baton;
  eb->called_open_root = FALSE;
  eb->base_revision = base_revision;
  eb->to_url = to_url;

  *editor = tree_editor;
  *edit_baton = eb;

  return SVN_NO_ERROR;
}

static svn_error_t *
do_synchronize (svn_ra_session_t *to_session, void *b, apr_pool_t *pool)
{
  svn_string_t *from_url, *from_uuid, *last_merged_rev;
  svn_revnum_t from_latest, current;
  svn_ra_session_t *from_session;
  sync_baton_t *baton = b;
  apr_pool_t *subpool;
  const char *uuid;

  /* First, figure out where we're supposed to pull from and how much we
   * should pull. */

  SVN_ERR (svn_ra_rev_prop (to_session, 0, FROM_URL_PROP, &from_url, pool));

  SVN_ERR (svn_ra_rev_prop (to_session, 0, FROM_UUID_PROP, &from_uuid, pool));

  SVN_ERR (svn_ra_rev_prop (to_session, 0, LAST_MERGED_REV_PROP,
                            &last_merged_rev, pool));

  if (! from_url || ! from_uuid || ! last_merged_rev)
    return svn_error_create
      (APR_EINVAL, NULL, "destination repository has not been initialized");

  SVN_ERR (svn_ra_open2 (&from_session, from_url->data, baton->callbacks,
                         baton, baton->config, pool));

  SVN_ERR (check_if_session_is_at_repos_root (from_session, from_url->data,
                                              pool));

  /* Ok, now sanity check the UUID of the source repository, it wouldn't
   * be a good thing to sync from a different repository. */

  SVN_ERR (svn_ra_get_uuid (from_session, &uuid, pool));

  if (strcmp (uuid, from_uuid->data) != 0)
    return svn_error_createf (APR_EINVAL, NULL,
                              "UUID of destination repository (%s) does not "
                              "match expected UUID (%s)",
                              uuid, from_uuid->data);

  /* Now check to see if there are any revisions to copy. */

  SVN_ERR (svn_ra_get_latest_revnum (from_session, &from_latest, pool));

  /* XXX see if we had a copy in progress that needs to be finished. */

  if (from_latest < atol (last_merged_rev->data))
    return SVN_NO_ERROR;

  subpool = svn_pool_create (pool);

  /* Ok, so there are new revisions, iterate over them copying them into
   * the destination repository. */

  for (current = atol (last_merged_rev->data) + 1;
       current <= from_latest;
       ++current)
    {
      const svn_delta_editor_t *commit_editor;
      const svn_delta_editor_t *cancel_editor;
      const svn_delta_editor_t *sync_editor;
      apr_hash_t *revprops;
      apr_hash_index_t *hi;
      void *commit_baton;
      void *cancel_baton;
      void *sync_baton;

      svn_pool_clear (subpool);

      /* The actual copy is just a diff hooked up to a commit. */

      SVN_ERR (svn_ra_get_commit_editor2 (to_session, &commit_editor,
                                          &commit_baton,
                                          "", /* empty log */
                                          commit_callback, baton,
                                          NULL, FALSE, subpool));

      /* There's one catch though, the diff shows us props we can't send
       * over the RA interface, so we need an editor that's smart enough
       * to filter those out for us. */
      SVN_ERR (get_sync_editor (commit_editor, commit_baton, current - 1,
                                baton->to_url, &sync_editor, &sync_baton,
                                subpool));

      SVN_ERR (svn_delta_get_cancellation_editor (check_cancel, NULL,
                                                  sync_editor, sync_baton,
                                                  &cancel_editor,
                                                  &cancel_baton,
                                                  pool));

      /* We set this property so that if we error out for some reason we can
       * later determine where we were in the process of merging a revision.
       * If we had committed the change, but we hadn't finished copying the
       * revprops we need to know that, so we can go back and finish the job
       * before we move on. */
      SVN_ERR (svn_ra_change_rev_prop (to_session, 0, CURRENTLY_COPYING_PROP,
                                       svn_string_createf (subpool, "%ld",
                                                           current),
                                       subpool));

      SVN_ERR (svn_ra_replay (from_session, current, 0, TRUE,
                              cancel_editor, cancel_baton, subpool));

      /* XXX sanity check that we just committed a change that resulted in
       * the revision number we expected, if it didn't, that means that the
       * two repositories are now out of sync, probably because someone has
       * accidentally committed a change to the destination repository. */

      /* Ok, we're done with the data, now we just need to do the revprops
       * and we're all set. */

      SVN_ERR (svn_ra_rev_proplist (from_session, current, &revprops,
                                    subpool));

      for (hi = apr_hash_first (subpool, revprops);
           hi;
           hi = apr_hash_next (hi))
        {
          const char *pname;
          svn_string_t *pval;
          const void *key;
          void *val;

          apr_hash_this (hi, &key, NULL, &val);

          pname = key;
          pval = val;

          SVN_ERR (svn_ra_change_rev_prop (to_session, current, pname, pval,
                                           subpool));
        }

      /* Ok, we're done, bring the last-merged-rev property up to date. */

      SVN_ERR (svn_ra_change_rev_prop
                 (to_session,
                  0,
                  LAST_MERGED_REV_PROP,
                  svn_string_create (apr_psprintf (subpool, "%ld", current),
                                     subpool),
                  subpool));

      /* And finally drop the currently copying prop, since we're done with
       * this revision. */

      SVN_ERR (svn_ra_change_rev_prop (to_session, 0, CURRENTLY_COPYING_PROP,
                                       NULL, subpool));
    }

  return SVN_NO_ERROR;
}

static svn_error_t *
synchronize (apr_getopt_t *os, void *b, apr_pool_t *pool)
{
  svn_ra_callbacks2_t callbacks = { 0 };
  svn_ra_session_t *to_session;
  opt_baton_t *opt_baton = b;
  apr_array_header_t *args;
  sync_baton_t baton;
  const char *to_url;

  SVN_ERR (svn_opt_parse_num_args (&args, os, 1, pool));

  to_url = APR_ARRAY_IDX (args, 0, const char *);

  callbacks.open_tmp_file = open_tmp_file;
  callbacks.auth_baton = opt_baton->auth_baton;

  baton.callbacks = &callbacks;
  baton.config = opt_baton->config;
  baton.to_url = to_url;

  SVN_ERR (svn_ra_open2 (&to_session,
                         to_url,
                         baton.callbacks,
                         &baton,
                         baton.config,
                         pool));

  SVN_ERR (check_if_session_is_at_repos_root (to_session, to_url, pool));

  SVN_ERR (with_locked (to_session, do_synchronize, &baton, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *
help (apr_getopt_t *os, void *baton, apr_pool_t *pool)
{
  opt_baton_t *opt_baton = baton;

  const char *header =
    _("general usage: svnsync SUBCOMMAND DEST_URL  [ARGS & OPTIONS ...]\n"
      "Type 'svnsync help <subcommand>' for help on a specific subcommand.\n"
      "\n"
      "Available subcommands:\n");

  const char *ra_desc_start
    = _("The following repository access (RA) modules are available:\n\n");

  svn_stringbuf_t *version_footer = svn_stringbuf_create (ra_desc_start,
                                                          pool);

  SVN_ERR (svn_ra_print_modules (version_footer, pool));

  SVN_ERR (svn_opt_print_help (os, "svnsync",
                               opt_baton ? opt_baton->version : FALSE,
                               FALSE, version_footer->data, header,
                               svnsync_cmd_table, svnsync_options, NULL,
                               pool));

  return SVN_NO_ERROR;
}

int
main (int argc, const char * const argv[])
{
  const svn_opt_subcommand_desc_t *subcommand = NULL;
  apr_array_header_t *received_opts;
  opt_baton_t opt_baton;
  svn_config_t *config;
  apr_status_t apr_err;
  apr_getopt_t *os;
  apr_pool_t *pool;
  svn_error_t *err;
  int opt_id, i;

  if (svn_cmdline_init ("svnsync", stderr) != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }

  err = check_lib_versions ();
  if (err)
    {
      svn_handle_error2 (err, stderr, FALSE, "svnsync: ");

      return EXIT_FAILURE;
    }

  pool = svn_pool_create (NULL);

  err = svn_ra_initialize (pool);
  if (err)
    {
      svn_handle_error2 (err, stderr, FALSE, "svnsync: ");

      return EXIT_FAILURE;
    }

  memset (&opt_baton, 0, sizeof (opt_baton));

  received_opts = apr_array_make (pool, SVN_OPT_MAX_OPTIONS, sizeof (int));

  if (argc <= 1)
    {
      help (NULL, NULL, pool);
      svn_pool_destroy (pool);
      return EXIT_FAILURE;
    }

  apr_getopt_init (&os, pool, argc, argv);

  os->interleave = 1;

  for (;;)
    {
      const char *opt_arg;

      apr_err = apr_getopt_long (os, svnsync_options, &opt_id, &opt_arg);
      if (APR_STATUS_IS_EOF (apr_err))
        break;
      else if (apr_err)
        {
          help (NULL, NULL, pool);
          svn_pool_destroy (pool);
          return EXIT_FAILURE;
        }

      APR_ARRAY_PUSH (received_opts, int) = opt_id;

      switch (opt_id)
        {
          case svnsync_opt_source_url:
            opt_baton.source_url = opt_arg;
            break;
          case svnsync_opt_non_interactive:
            opt_baton.non_interactive = TRUE;
            break;

          case svnsync_opt_no_auth_cache:
            opt_baton.no_auth_cache = TRUE;
            break;

          case svnsync_opt_auth_username:
            opt_baton.auth_username = opt_arg;
            break;

          case svnsync_opt_auth_password:
            opt_baton.auth_password = opt_arg;
            break;

          case svnsync_opt_config_dir:
            opt_baton.config_dir = opt_arg;
            break;

          case svnsync_opt_version:
            opt_baton.version = TRUE;
            opt_baton.help = TRUE;
            break;

          case svnsync_opt_help:
            opt_baton.help = TRUE;
            break;

          default:
            {
              help (NULL, NULL, pool);
              svn_pool_destroy (pool);
              return EXIT_FAILURE;
            }
        }
    }

  if (opt_baton.help)
    subcommand = svn_opt_get_canonical_subcommand (svnsync_cmd_table, "help");

  if (subcommand == NULL)
    {
      if (os->ind >= os->argc)
        {
          help (NULL, NULL, pool);
          svn_pool_destroy (pool);
          return EXIT_FAILURE;
        }
      else
        {
          const char *first_arg = os->argv[os->ind++];
          subcommand = svn_opt_get_canonical_subcommand (svnsync_cmd_table,
                                                         first_arg);
          if (subcommand == NULL)
            {
              help (NULL, NULL, pool);
              svn_pool_destroy (pool);
              return EXIT_FAILURE;
            }
        }
    }

  for (i = 0; i < received_opts->nelts; ++i)
    {
      opt_id = APR_ARRAY_IDX (received_opts, i, int);

      if (opt_id == 'h' || opt_id == '?')
        continue;

      if (! svn_opt_subcommand_takes_option (subcommand, opt_id))
        {
          const char *optstr;
          const apr_getopt_option_t *badopt =
            svn_opt_get_option_from_code (opt_id, svnsync_options);
          svn_opt_format_option (&optstr, badopt, FALSE, pool);
          svn_error_clear
            (svn_cmdline_fprintf
             (stderr, pool, _("subcommand '%s' doesn't accept option '%s'\n"
                              "Type 'svnsync help %s' for usage.\n"),
              subcommand->name, optstr, subcommand->name));
          svn_pool_destroy (pool);
          return EXIT_FAILURE;
        }
    }

  err = svn_config_get_config (&opt_baton.config, NULL, pool);
  if (err)
    return svn_cmdline_handle_exit_error (err, pool, "svnsync: ");

  config = apr_hash_get (opt_baton.config, SVN_CONFIG_CATEGORY_CONFIG,
                         APR_HASH_KEY_STRING);

  apr_signal (SIGINT, signal_handler);

#ifdef SIGBREAK
  /* SIGBREAK is a Win32 specific signal generated by ctrl-break. */
  apr_signal (SIGBREAK, signal_handler);
#endif

#ifdef SIGHUP
  apr_signal (SIGHUP, signal_handler);
#endif

#ifdef SIGTERM
  apr_signal (SIGTERM, signal_handler);
#endif

#ifdef SIGPIPE
  /* Disable SIGPIPE generation for the platforms that have it. */
  apr_signal(SIGPIPE, SIG_IGN);
#endif

  err = svn_cmdline_setup_auth_baton (&opt_baton.auth_baton,
                                      opt_baton.non_interactive,
                                      opt_baton.auth_username,
                                      opt_baton.auth_password,
                                      opt_baton.config_dir,
                                      opt_baton.no_auth_cache,
                                      config,
                                      check_cancel, NULL,
                                      pool);

  err = (*subcommand->cmd_func) (os, &opt_baton, pool);
  if (err)
    {
      svn_handle_error2 (err, stderr, FALSE, "svnsync: ");

      return EXIT_FAILURE;
    }

  svn_pool_destroy (pool);

  return EXIT_SUCCESS;
}
