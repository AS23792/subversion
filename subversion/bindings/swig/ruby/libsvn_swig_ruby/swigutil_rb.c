
/* Tell swigutil_rb.h that we're inside the implementation */
#define SVN_SWIG_SWIGUTIL_RB_C

#include "rubyhead.swg"
#include "swig_ruby_external_runtime.swg"
#include "swigutil_rb.h"
#include <st.h>

#ifndef RE_OPTION_IGNORECASE
#  ifdef ONIG_OPTION_IGNORECASE
#    define RE_OPTION_IGNORECASE ONIG_OPTION_IGNORECASE
#  endif
#endif

#include <locale.h>

#include "svn_nls.h"
#include "svn_pools.h"
#include "svn_time.h"
#include "svn_utf.h"


#define AOFF2NUM(num) \
  (sizeof(apr_off_t) == SIZEOF_LONG_LONG ? LL2NUM(num) : LONG2NUM(num))

#if SIZEOF_LONG_LONG == 8
#  define AI642NUM(num) LL2NUM(num)
#else
#  define AI642NUM(num) LONG2NUM(num)
#endif

#define EMPTY_CPP_ARGUMENT

#define POOL_P(obj) (RTEST(rb_obj_is_kind_of(obj, rb_svn_core_pool())))
#define CONTEXT_P(obj) (RTEST(rb_obj_is_kind_of(obj, rb_svn_client_context())))
#define SVN_ERR_P(obj) (RTEST(rb_obj_is_kind_of(obj, rb_svn_error())))

static VALUE mSvn = Qnil;
static VALUE mSvnClient = Qnil;
static VALUE mSvnUtil = Qnil;
static VALUE cSvnClientContext = Qnil;
static VALUE mSvnCore = Qnil;
static VALUE cSvnCorePool = Qnil;
static VALUE cSvnCoreStream = Qnil;
static VALUE cSvnDelta = Qnil;
static VALUE cSvnDeltaEditor = Qnil;
static VALUE cSvnDeltaTextDeltaWindowHandler = Qnil;
static VALUE cSvnError = Qnil;
static VALUE cSvnErrorSvnError = Qnil;
static VALUE cSvnFs = Qnil;
static VALUE cSvnFsFileSystem = Qnil;

#define DEFINE_ID(key, name)                    \
static ID id_ ## key = 0;                       \
static ID                                       \
rb_id_ ## key(void)                             \
{                                               \
  if (!id_ ## key) {                            \
    id_ ## key = rb_intern(name);               \
  }                                             \
  return id_ ## key;                            \
}

DEFINE_ID(code, "code")
DEFINE_ID(message, "message")
DEFINE_ID(call, "call")
DEFINE_ID(read, "read")
DEFINE_ID(write, "write")
DEFINE_ID(eqq, "===")
DEFINE_ID(baton, "baton")
DEFINE_ID(new, "new")
DEFINE_ID(new_corresponding_error, "new_corresponding_error")
DEFINE_ID(set_target_revision, "set_target_revision")
DEFINE_ID(open_root, "open_root")
DEFINE_ID(delete_entry, "delete_entry")
DEFINE_ID(add_directory, "add_directory")
DEFINE_ID(open_directory, "open_directory")
DEFINE_ID(change_dir_prop, "change_dir_prop")
DEFINE_ID(close_directory, "close_directory")
DEFINE_ID(absent_directory, "absent_directory")
DEFINE_ID(add_file, "add_file")
DEFINE_ID(open_file, "open_file")
DEFINE_ID(apply_textdelta, "apply_textdelta")
DEFINE_ID(change_file_prop, "change_file_prop")
DEFINE_ID(absent_file, "absent_file")
DEFINE_ID(close_file, "close_file")
DEFINE_ID(close_edit, "close_edit")
DEFINE_ID(abort_edit, "abort_edit")
DEFINE_ID(__pool__, "__pool__")
DEFINE_ID(__pools__, "__pools__")
DEFINE_ID(name, "name")
DEFINE_ID(swig_type_regex, "swig_type_regex")
DEFINE_ID(open_tmp_file, "open_tmp_file")
DEFINE_ID(get_wc_prop, "get_wc_prop")
DEFINE_ID(set_wc_prop, "set_wc_prop")
DEFINE_ID(push_wc_prop, "push_wc_prop")
DEFINE_ID(invalidate_wc_props, "invalidate_wc_props")
DEFINE_ID(progress_func, "progress_func")
DEFINE_ID(auth_baton, "auth_baton")
DEFINE_ID(found_entry, "found_entry")
DEFINE_ID(file_changed, "file_changed")
DEFINE_ID(file_added, "file_added")
DEFINE_ID(file_deleted, "file_deleted")
DEFINE_ID(dir_added, "dir_added")
DEFINE_ID(dir_deleted, "dir_deleted")
DEFINE_ID(dir_props_changed, "dir_props_changed")
DEFINE_ID(handler, "handler")
DEFINE_ID(handler_baton, "handler_baton")
DEFINE_ID(__batons__, "__batons__")
DEFINE_ID(destroy, "destroy")
DEFINE_ID(filename_to_temp_file, "filename_to_temp_file")
DEFINE_ID(inspect, "inspect")

typedef void *(*r2c_func)(VALUE value, void *ctx, apr_pool_t *pool);
typedef VALUE (*c2r_func)(void *value, void *ctx);
typedef struct hash_to_apr_hash_data_t
{
  apr_hash_t *apr_hash;
  r2c_func func;
  void *ctx;
  apr_pool_t *pool;
} hash_to_apr_hash_data_t;

static void r2c_swig_type2(VALUE value, const char *type_name, void **result);



/* constant getter */
static VALUE
rb_svn(void)
{
  if (NIL_P(mSvn)) {
    mSvn = rb_const_get(rb_cObject, rb_intern("Svn"));
  }
  return mSvn;
}

static VALUE
rb_svn_util(void)
{
  if (NIL_P(mSvnUtil)) {
    mSvnUtil = rb_const_get(rb_svn(), rb_intern("Util"));
  }
  return mSvnUtil;
}

static VALUE
rb_svn_client(void)
{
  if (NIL_P(mSvnClient)) {
    mSvnClient = rb_const_get(rb_svn(), rb_intern("Client"));
  }
  return mSvnClient;
}

static VALUE
rb_svn_client_context(void)
{
  if (NIL_P(cSvnClientContext)) {
    cSvnClientContext = rb_const_get(rb_svn_client(), rb_intern("Context"));
  }
  return cSvnClientContext;
}

static VALUE
rb_svn_core(void)
{
  if (NIL_P(mSvnCore)) {
    mSvnCore = rb_const_get(rb_svn(), rb_intern("Core"));
  }
  return mSvnCore;
}

static VALUE
rb_svn_core_pool(void)
{
  if (NIL_P(cSvnCorePool)) {
    cSvnCorePool = rb_const_get(rb_svn_core(), rb_intern("Pool"));
    rb_ivar_set(cSvnCorePool, rb_id___pools__(), rb_hash_new());
  }
  return cSvnCorePool;
}

static VALUE
rb_svn_core_stream(void)
{
  if (NIL_P(cSvnCoreStream)) {
    cSvnCoreStream = rb_const_get(rb_svn_core(), rb_intern("Stream"));
  }
  return cSvnCoreStream;
}

static VALUE
rb_svn_delta(void)
{
  if (NIL_P(cSvnDelta)) {
    cSvnDelta = rb_const_get(rb_svn(), rb_intern("Delta"));
  }
  return cSvnDelta;
}

VALUE
svn_swig_rb_svn_delta_editor(void)
{
  if (NIL_P(cSvnDeltaEditor)) {
    cSvnDeltaEditor =
      rb_const_get(rb_svn_delta(), rb_intern("Editor"));
  }
  return cSvnDeltaEditor;
}

VALUE
svn_swig_rb_svn_delta_text_delta_window_handler(void)
{
  if (NIL_P(cSvnDeltaTextDeltaWindowHandler)) {
    cSvnDeltaTextDeltaWindowHandler =
      rb_const_get(rb_svn_delta(), rb_intern("TextDeltaWindowHandler"));
  }
  return cSvnDeltaTextDeltaWindowHandler;
}

static VALUE
rb_svn_error(void)
{
  if (NIL_P(cSvnError)) {
    cSvnError = rb_const_get(rb_svn(), rb_intern("Error"));
  }
  return cSvnError;
}

static VALUE
rb_svn_error_svn_error(void)
{
  if (NIL_P(cSvnErrorSvnError)) {
    cSvnErrorSvnError = rb_const_get(rb_svn_error(), rb_intern("SvnError"));
  }
  return cSvnErrorSvnError;
}

static VALUE
rb_svn_fs(void)
{
  if (NIL_P(cSvnFs)) {
    cSvnFs = rb_const_get(rb_svn(), rb_intern("Fs"));
  }
  return cSvnFs;
}

static VALUE
rb_svn_fs_file_system(void)
{
  if (NIL_P(cSvnFsFileSystem)) {
    cSvnFsFileSystem = rb_const_get(rb_svn_fs(), rb_intern("FileSystem"));
    rb_ivar_set(cSvnFsFileSystem, rb_id___batons__(), rb_hash_new());
  }
  return cSvnFsFileSystem;
}


/* initialize */
static VALUE
svn_swig_rb_converter_to_locale_encoding(VALUE self, VALUE str)
{
  apr_pool_t *pool;
  svn_error_t *err;
  const char *dest;
  VALUE result;

  pool = svn_pool_create(NULL);
  err = svn_utf_cstring_from_utf8(&dest, StringValueCStr(str), pool);
  if (err) {
    apr_pool_destroy(pool);
    svn_swig_rb_handle_svn_error(err);
  }

  result = rb_str_new2(dest);
  apr_pool_destroy(pool);
  return result;
}

static VALUE
svn_swig_rb_locale_set(int argc, VALUE *argv, VALUE self)
{
  char *result;
  int category;
  const char *locale;
  VALUE rb_category, rb_locale;

  rb_scan_args(argc, argv, "02", &rb_category, &rb_locale);

  if (NIL_P(rb_category))
    category = LC_ALL;
  else
    category = NUM2INT(rb_category);

  if (NIL_P(rb_locale))
    locale = "";
  else
    locale = StringValueCStr(rb_locale);

  result = setlocale(category, locale);

  return result ? rb_str_new2(result) : Qnil;
}

void
svn_swig_rb_initialize(void)
{
  apr_status_t status;
  apr_pool_t *pool;
  VALUE mSvnConverter, mSvnLocale;

  status = apr_initialize();
  if (status) {
    char buf[1024];
    apr_strerror(status, buf, sizeof(buf) - 1);
    rb_raise(rb_eLoadError, "cannot initialize APR: %s", buf);
  }

  if (atexit(apr_terminate)) {
    rb_raise(rb_eLoadError, "atexit registration failed");
  }

  pool = svn_pool_create(NULL);
  svn_utf_initialize(pool);

  mSvnConverter = rb_define_module_under(rb_svn(), "Converter");
  rb_define_module_function(mSvnConverter, "to_locale_encoding",
                            svn_swig_rb_converter_to_locale_encoding, 1);

  mSvnLocale = rb_define_module_under(rb_svn(), "Locale");
  rb_define_const(mSvnLocale, "ALL", INT2NUM(LC_ALL));
  rb_define_const(mSvnLocale, "COLLATE", INT2NUM(LC_COLLATE));
  rb_define_const(mSvnLocale, "CTYPE", INT2NUM(LC_CTYPE));
#ifdef LC_MESSAGES
  rb_define_const(mSvnLocale, "MESSAGES", INT2NUM(LC_MESSAGES));
#endif
  rb_define_const(mSvnLocale, "MONETARY", INT2NUM(LC_MONETARY));
  rb_define_const(mSvnLocale, "NUMERIC", INT2NUM(LC_NUMERIC));
  rb_define_const(mSvnLocale, "TIME", INT2NUM(LC_TIME));
  rb_define_module_function(mSvnLocale, "set", svn_swig_rb_locale_set, -1);
}


/* pool holder */
static VALUE
rb_svn_pool_holder(void)
{
  return rb_ivar_get(rb_svn_core_pool(), rb_id___pools__());
}

static VALUE
rb_svn_fs_warning_callback_baton_holder(void)
{
  return rb_ivar_get(rb_svn_fs_file_system(), rb_id___batons__());
}

static VALUE
rb_holder_push(VALUE holder, VALUE obj)
{
  VALUE key, objs;

  key = rb_obj_id(obj);
  objs = rb_hash_aref(holder, key);

  if (NIL_P(objs)) {
    objs = rb_ary_new();
    rb_hash_aset(holder, key, objs);
  }

  rb_ary_push(objs, obj);

  return Qnil;
}

static VALUE
rb_holder_pop(VALUE holder, VALUE obj)
{
  VALUE key, objs;
  VALUE result = Qnil;

  key = rb_obj_id(obj);
  objs = rb_hash_aref(holder, key);

  if (!NIL_P(objs)) {
    result = rb_ary_pop(objs);
    if (RARRAY(objs)->len == 0) {
      rb_hash_delete(holder, key);
    }
  }

  return result;
}


/* pool */
static VALUE
rb_get_pool(VALUE self)
{
  return rb_ivar_get(self, rb_id___pool__());
}

static VALUE
rb_pools(VALUE self)
{
  VALUE pools = rb_ivar_get(self, rb_id___pools__());

  if (NIL_P(pools)) {
    pools = rb_hash_new();
    rb_ivar_set(self, rb_id___pools__(), pools);
  }
  
  return pools;
}

static VALUE
rb_set_pool(VALUE self, VALUE pool)
{
  if (NIL_P(pool)) {
    VALUE old_pool = rb_ivar_get(self, rb_id___pool__());
    rb_hash_aset(rb_pools(self), rb_obj_id(old_pool), old_pool);
    rb_ivar_set(self, rb_id___pool__(), Qnil);
  } else {
    if (NIL_P(rb_ivar_get(self, rb_id___pool__()))) {
      rb_ivar_set(self, rb_id___pool__(), pool);
    } else {
      rb_hash_aset(rb_pools(self), rb_obj_id(pool), pool);
    }
  }

  return Qnil;
}

static VALUE
rb_pool_new(VALUE parent)
{
  return rb_funcall(rb_svn_core_pool(), rb_id_new(), 1, parent);
}

static VALUE swig_type_re = Qnil;

static VALUE
swig_type_regex(void)
{
  if (NIL_P(swig_type_re)) {
    char reg_str[] = "\\A(?:SWIG|Svn::Ext)::";
    swig_type_re = rb_reg_new(reg_str, strlen(reg_str), 0);
    rb_ivar_set(rb_svn(), rb_id_swig_type_regex(), swig_type_re);
  }
  return swig_type_re;
}

static VALUE
find_swig_type_object(int num, VALUE *objects)
{
  VALUE re = swig_type_regex();
  int i;

  for (i = 0; i < num; i++) {
    if (RTEST(rb_reg_match(re,
                           rb_funcall(rb_obj_class(objects[i]),
                                      rb_id_name(),
                                      0)))) {
      return objects[i];
    }
  }
  
  return Qnil;
}

void
svn_swig_rb_get_pool(int argc, VALUE *argv, VALUE self,
                     VALUE *rb_pool, apr_pool_t **pool)
{
  *rb_pool = Qnil;
  
  if (argc > 0) {
    if (POOL_P(argv[argc - 1])) {
      *rb_pool = rb_pool_new(argv[argc - 1]);
      argc -= 1;
    }
  }

  if (NIL_P(*rb_pool) && !NIL_P(self)) {
    *rb_pool = rb_get_pool(self);
    if (POOL_P(*rb_pool)) {
      *rb_pool = rb_pool_new(*rb_pool);
    } else {
      *rb_pool = Qnil;
    }
  }

  if (NIL_P(*rb_pool)) {
    VALUE target;
    target = find_swig_type_object(argc, argv);
    *rb_pool = rb_pool_new(rb_get_pool(target));
  }

  if (pool) {
    apr_pool_wrapper_t *pool_wrapper;
    apr_pool_wrapper_t **pool_wrapper_p;

    pool_wrapper_p = &pool_wrapper;
    r2c_swig_type2(*rb_pool, "apr_pool_wrapper_t *", (void **)pool_wrapper_p);
    *pool = pool_wrapper->pool;
  }
}

static svn_boolean_t
rb_set_pool_if_swig_type_object(VALUE target, VALUE pool)
{
  VALUE targets[1] = {target};

  if (!NIL_P(find_swig_type_object(1, targets))) {
    rb_set_pool(target, pool);
    return TRUE;
  } else {
    return FALSE;
  }
}

struct rb_set_pool_for_hash_arg {
  svn_boolean_t set;
  VALUE pool;
};

static int
rb_set_pool_for_hash_callback(VALUE key, VALUE value,
                              struct rb_set_pool_for_hash_arg *arg)
{
  if (rb_set_pool_if_swig_type_object(value, arg->pool))
    arg->set = TRUE;
  return ST_CONTINUE;
}

svn_boolean_t
svn_swig_rb_set_pool(VALUE target, VALUE pool)
{
  if (NIL_P(target)) {
    return FALSE;
  }

  if (RTEST(rb_obj_is_kind_of(target, rb_cArray))) {
    long i;
    svn_boolean_t set = FALSE;

    for (i = 0; i < RARRAY(target)->len; i++) {
      if (rb_set_pool_if_swig_type_object(RARRAY(target)->ptr[i], pool))
        set = TRUE;
    }
    return set;
  } else if (RTEST(rb_obj_is_kind_of(target, rb_cHash))) {
    struct rb_set_pool_for_hash_arg arg;
    arg.set = FALSE;
    arg.pool = pool;
    rb_hash_foreach(target, rb_set_pool_for_hash_callback, (VALUE)&arg);
    return arg.set;
  } else {
    return rb_set_pool_if_swig_type_object(target, pool);
  }
}

void
svn_swig_rb_set_pool_for_no_swig_type(VALUE target, VALUE pool)
{
  if (NIL_P(target)) {
    return;
  }
    
  if (!RTEST(rb_obj_is_kind_of(target, rb_cArray))) {
    target = rb_ary_new3(1, target);
  }

  rb_iterate(rb_each, target, rb_set_pool, pool);
}

void
svn_swig_rb_push_pool(VALUE pool)
{
  if (!NIL_P(pool)) {
    rb_holder_push(rb_svn_pool_holder(), pool);
  }
}

void
svn_swig_rb_pop_pool(VALUE pool)
{
  if (!NIL_P(pool)) {
    rb_holder_pop(rb_svn_pool_holder(), pool);
  }
}

void
svn_swig_rb_destroy_pool(VALUE pool)
{
  if (!NIL_P(pool)) {
    rb_funcall(pool, rb_id_destroy(), 0);
  }
}

void
svn_swig_rb_destroy_internal_pool(VALUE object)
{
  svn_swig_rb_destroy_pool(rb_get_pool(object));
}


/* error */
void
svn_swig_rb_raise_svn_fs_already_close(void)
{
  static VALUE rb_svn_error_fs_already_close = 0;

  if (!rb_svn_error_fs_already_close) {
    rb_svn_error_fs_already_close =
      rb_const_get(rb_svn_error(), rb_intern("FsAlreadyClose"));
  }

  rb_raise(rb_svn_error_fs_already_close, "closed file system");
}

void
svn_swig_rb_raise_svn_repos_already_close(void)
{
  static VALUE rb_svn_error_repos_already_close = 0;

  if (!rb_svn_error_repos_already_close) {
    rb_svn_error_repos_already_close =
      rb_const_get(rb_svn_error(), rb_intern("ReposAlreadyClose"));
  }

  rb_raise(rb_svn_error_repos_already_close, "closed repository");
}

VALUE
svn_swig_rb_svn_error_new(VALUE code, VALUE message, VALUE file, VALUE line)
{
  return rb_funcall(rb_svn_error_svn_error(),
                    rb_id_new_corresponding_error(),
                    4, code, message, file, line);
}

static VALUE
svn_swig_rb_svn_error_to_rb_error(svn_error_t *error)
{
  VALUE error_code = INT2NUM(error->apr_err);
  VALUE message;
  VALUE file = Qnil;
  VALUE line = Qnil;

  if (error->file)
    file = rb_str_new2(error->file);
  if (error->line)
    line = LONG2NUM(error->line);
  
  message = rb_str_new2(error->message ? error->message : "");
  
  while (error->child) {
    error = error->child;
    if (error->message) {
      rb_str_concat(message, rb_str_new2("\n"));
      rb_str_concat(message, rb_str_new2(error->message));
    }
  }

  return svn_swig_rb_svn_error_new(error_code, message, file, line);
}

void
svn_swig_rb_handle_svn_error(svn_error_t *error)
{
  VALUE rb_error = svn_swig_rb_svn_error_to_rb_error(error);
  svn_error_clear(error);
  rb_exc_raise(rb_error);
}


static VALUE inited = Qnil;
/* C -> Ruby */
VALUE
svn_swig_rb_from_swig_type(void *value, void *ctx)
{
  swig_type_info *info;

  if (NIL_P(inited)) {
    SWIG_InitRuntime();
    inited = Qtrue;
  }
  
  info = SWIG_TypeQuery((char *)ctx);
  if (info) {
    return SWIG_NewPointerObj(value, info, 0);
  } else {
    rb_raise(rb_eArgError, "invalid SWIG type: %s", (char *)ctx);
  }
}
#define c2r_swig_type svn_swig_rb_from_swig_type

static VALUE
c2r_string(void *value, void *ctx)
{
  if (value) {
    return rb_str_new2((const char *)value);
  } else {
    return Qnil;
  }
}

static VALUE
c2r_string2(const char *cstr)
{
  return c2r_string((void *)cstr, NULL);
}

VALUE
svn_swig_rb_svn_date_string_to_time(const char *date)
{
  if (date) {
    apr_time_t tm;
    svn_error_t *error;
    apr_pool_t *pool;

    pool = svn_pool_create(NULL);
    error = svn_time_from_cstring(&tm, date, pool);
    svn_pool_destroy(pool);
    if (error)
      svn_swig_rb_handle_svn_error(error);
    return rb_time_new(apr_time_sec(tm), apr_time_usec(tm));
  } else {
    return Qnil;
  }
}
#define c2r_svn_date_string2 svn_swig_rb_svn_date_string_to_time

static VALUE
c2r_long(void *value, void *ctx)
{
  return INT2NUM(*(long *)value);
}

static VALUE
c2r_svn_string(void *value, void *ctx)
{
  const svn_string_t *s = (svn_string_t *)value;

  return c2r_string2(s->data);
}


/* C -> Ruby (dup) */
#define DEFINE_DUP_BASE(type, dup_func, type_prefix)                         \
static VALUE                                                                 \
c2r_ ## type ## _dup(void *type, void *ctx)                                  \
{                                                                            \
  apr_pool_t *pool;                                                          \
  VALUE rb_pool;                                                             \
  svn_ ## type ## _t *copied_item;                                           \
  VALUE rb_copied_item;                                                      \
                                                                             \
  if (!type)                                                                 \
    return Qnil;                                                             \
                                                                             \
  svn_swig_rb_get_pool(0, (VALUE *)0, 0, &rb_pool, &pool);                   \
  copied_item = svn_ ## dup_func((type_prefix svn_ ## type ## _t *)type,     \
                                  pool);                                     \
  rb_copied_item = c2r_swig_type((void *)copied_item,                        \
                                 (void *)"svn_" # type "_t *");              \
  rb_set_pool(rb_copied_item, rb_pool);                                      \
                                                                             \
  return rb_copied_item;                                                     \
}

#define DEFINE_DUP_BASE_WITH_CONVENIENCE(type, dup_func, type_prefix)        \
DEFINE_DUP_BASE(type, dup_func, type_prefix)                                 \
static VALUE                                                                 \
c2r_ ## type ## __dup(type_prefix svn_ ## type ## _t *type)                  \
{                                                                            \
  return c2r_ ## type ## _dup((void *)type, NULL);                           \
}

#define DEFINE_DUP(type, dup_func) \
  DEFINE_DUP_BASE_WITH_CONVENIENCE(type, dup_func, const)
#define DEFINE_DUP2(type) \
  DEFINE_DUP(type, type ## _dup)

#define DEFINE_DUP_NO_CONVENIENCE(type, dup_func) \
  DEFINE_DUP_BASE(type, dup_func, const)
#define DEFINE_DUP_NO_CONVENIENCE2(type) \
  DEFINE_DUP_NO_CONVENIENCE(type, type ## _dup)

#define DEFINE_DUP_NO_CONST(type, dup_func) \
  DEFINE_DUP_BASE_WITH_CONVENIENCE(type, dup_func,)
#define DEFINE_DUP_NO_CONST2(type) \
  DEFINE_DUP_NO_CONST(type, type ## _dup)


DEFINE_DUP(wc_notify, wc_dup_notify)
DEFINE_DUP2(txdelta_window)
DEFINE_DUP2(info)
DEFINE_DUP2(commit_info)
DEFINE_DUP2(lock)
DEFINE_DUP2(auth_ssl_server_cert_info)
DEFINE_DUP2(wc_entry)
DEFINE_DUP2(client_diff_summarize)
DEFINE_DUP2(dirent)
DEFINE_DUP_NO_CONVENIENCE2(prop)
DEFINE_DUP_NO_CONVENIENCE2(client_commit_item3)
DEFINE_DUP_NO_CONVENIENCE2(client_proplist_item)
DEFINE_DUP_NO_CONVENIENCE2(wc_external_item)
DEFINE_DUP_NO_CONVENIENCE2(log_changed_path)
DEFINE_DUP_NO_CONST(wc_status2, wc_dup_status2)


/* Ruby -> C */
static const char *
r2c_inspect(VALUE object)
{
  VALUE inspected;
  inspected = rb_funcall(object, rb_id_inspect(), 0);
  return StringValueCStr(inspected);
}

static void *
r2c_string(VALUE value, void *ctx, apr_pool_t *pool)
{
  return (void *)apr_pstrdup(pool, StringValuePtr(value));
}

static void *
r2c_svn_string(VALUE value, void *ctx, apr_pool_t *pool)
{
  return (void *)svn_string_create(StringValuePtr(value), pool);
}

void *
svn_swig_rb_to_swig_type(VALUE value, void *ctx, apr_pool_t *pool)
{
  void **result = NULL;
  result = apr_palloc(pool, sizeof(void *));
  r2c_swig_type2(value, (const char *)ctx, result);
  return *result;
}
#define r2c_swig_type svn_swig_rb_to_swig_type

static void
r2c_swig_type2(VALUE value, const char *type_name, void **result)
{
  int res;
  res = SWIG_ConvertPtr(value, result, SWIG_TypeQuery(type_name),
                        SWIG_POINTER_EXCEPTION);
#ifdef SWIG_IsOK
  if (!SWIG_IsOK(res)) {
    VALUE message = rb_funcall(value, rb_intern("inspect"), 0);
    rb_str_cat2(message, "must be ");
    rb_str_cat2(message, type_name);
    SWIG_Error(SWIG_ArgError(res), StringValuePtr(message));
  }
#endif
}

static void *
r2c_long(VALUE value, void *ctx, apr_pool_t *pool)
{
  return (void *)NUM2LONG(value);
}

static void *
r2c_svn_err(VALUE rb_svn_err, void *ctx, apr_pool_t *pool)
{
  VALUE message;
  svn_error_t *err;

  message = rb_funcall(rb_svn_err, rb_id_message(), 0);
  err = svn_error_create(NUM2INT(rb_funcall(rb_svn_err, rb_id_code(), 0)),
                         NULL,
                         StringValuePtr(message));
  return (void *)err;
}


/* apr_array_t -> Ruby Array */
#define DEFINE_APR_ARRAY_TO_ARRAY(return_type, name, conv, amp, type, ctx)  \
return_type                                                                 \
name(const apr_array_header_t *apr_ary)                                     \
{                                                                           \
  VALUE ary = rb_ary_new();                                                 \
  int i;                                                                    \
                                                                            \
  for (i = 0; i < apr_ary->nelts; i++) {                                    \
    rb_ary_push(ary, conv((void *)amp(APR_ARRAY_IDX(apr_ary, i, type)),     \
                          ctx));                                            \
  }                                                                         \
                                                                            \
  return ary;                                                               \
}

DEFINE_APR_ARRAY_TO_ARRAY(VALUE, svn_swig_rb_apr_array_to_array_string,
                          c2r_string, EMPTY_CPP_ARGUMENT, const char *, NULL)
DEFINE_APR_ARRAY_TO_ARRAY(VALUE, svn_swig_rb_apr_array_to_array_svn_string,
                          c2r_svn_string, &, svn_string_t, NULL)
DEFINE_APR_ARRAY_TO_ARRAY(static VALUE, c2r_commit_item3_array,
                          c2r_client_commit_item3_dup, EMPTY_CPP_ARGUMENT,
                          svn_client_commit_item3_t *, NULL)
DEFINE_APR_ARRAY_TO_ARRAY(VALUE, svn_swig_rb_apr_array_to_array_prop,
                          c2r_prop_dup, &, svn_prop_t, NULL)
DEFINE_APR_ARRAY_TO_ARRAY(VALUE, svn_swig_rb_apr_array_to_array_svn_rev,
                          c2r_long, &, svn_revnum_t, NULL)
DEFINE_APR_ARRAY_TO_ARRAY(VALUE, svn_swig_rb_apr_array_to_array_proplist_item,
                          c2r_client_proplist_item_dup, EMPTY_CPP_ARGUMENT,
                          svn_client_proplist_item_t *, NULL)
DEFINE_APR_ARRAY_TO_ARRAY(VALUE, svn_swig_rb_apr_array_to_array_external_item,
                          c2r_wc_external_item_dup, EMPTY_CPP_ARGUMENT,
                          svn_wc_external_item_t *, NULL)


/* Ruby Array -> apr_array_t */
#define DEFINE_ARRAY_TO_APR_ARRAY(type, name, converter, context) \
apr_array_header_t *                                              \
name(VALUE array, apr_pool_t *pool)                               \
{                                                                 \
  int i, len;                                                     \
  apr_array_header_t *apr_ary;                                    \
                                                                  \
  Check_Type(array, T_ARRAY);                                     \
  len = RARRAY(array)->len;                                       \
  apr_ary = apr_array_make(pool, len, sizeof(type));              \
  apr_ary->nelts = len;                                           \
  for (i = 0; i < len; i++) {                                     \
    VALUE value;                                                  \
    type val;                                                     \
    value = rb_ary_entry(array, i);                               \
    val = (type)converter(value, context, pool);                  \
    APR_ARRAY_IDX(apr_ary, i, type) = val;                        \
  }                                                               \
  return apr_ary;                                                 \
}

DEFINE_ARRAY_TO_APR_ARRAY(const char *, svn_swig_rb_strings_to_apr_array,
                          r2c_string, NULL)
DEFINE_ARRAY_TO_APR_ARRAY(svn_auth_provider_object_t *,
                          svn_swig_rb_array_to_auth_provider_object_apr_array,
                          r2c_swig_type, (void *)"svn_auth_provider_object_t *")
DEFINE_ARRAY_TO_APR_ARRAY(svn_prop_t *,
                          svn_swig_rb_array_to_apr_array_prop,
                          r2c_swig_type, (void *)"svn_prop_t *")
DEFINE_ARRAY_TO_APR_ARRAY(svn_revnum_t,
                          svn_swig_rb_array_to_apr_array_revnum,
                          r2c_long, NULL)
DEFINE_ARRAY_TO_APR_ARRAY(svn_merge_range_t *,
                          svn_swig_rb_array_to_apr_array_merge_range,
                          r2c_swig_type, (void *)"svn_merge_range_t *")


/* apr_hash_t -> Ruby Hash */
static VALUE
c2r_hash_with_key_convert(apr_hash_t *hash,
                          c2r_func key_conv,
                          void *key_ctx,
                          c2r_func value_conv,
                          void *value_ctx)
{
  apr_hash_index_t *hi;
  VALUE r_hash;

  if (!hash)
    return Qnil;

  r_hash = rb_hash_new();

  for (hi = apr_hash_first(NULL, hash); hi; hi = apr_hash_next(hi)) {
    const void *key;
    void *val;
    VALUE v = Qnil;
    
    apr_hash_this(hi, &key, NULL, &val);
    if (val) {
      v = (*value_conv)(val, value_ctx);
    }
    rb_hash_aset(r_hash, (*key_conv)((void *)key, key_ctx), v);
  }
  
  return r_hash;
}

VALUE
c2r_hash(apr_hash_t *hash,
         c2r_func value_conv,
         void *ctx)
{
  return c2r_hash_with_key_convert(hash, c2r_string, NULL, value_conv, ctx);
}

VALUE
svn_swig_rb_apr_hash_to_hash_string(apr_hash_t *hash)
{
  return c2r_hash(hash, c2r_string, NULL);
}

VALUE
svn_swig_rb_apr_hash_to_hash_svn_string(apr_hash_t *hash)
{
  return c2r_hash(hash, c2r_svn_string, NULL);
}

VALUE
svn_swig_rb_apr_hash_to_hash_swig_type(apr_hash_t *hash, const char *type_name)
{
  return c2r_hash(hash, c2r_swig_type, (void *)type_name);
}

VALUE
svn_swig_rb_prop_hash_to_hash(apr_hash_t *prop_hash)
{
  return svn_swig_rb_apr_hash_to_hash_svn_string(prop_hash);
}

VALUE
c2r_revnum(void *value, void *ctx)
{
  svn_revnum_t *num = value;
  return INT2NUM(*num);
}

VALUE
svn_swig_rb_apr_revnum_key_hash_to_hash_string(apr_hash_t *hash)
{
  return c2r_hash_with_key_convert(hash, c2r_revnum, NULL, c2r_string, NULL);
}


/* Ruby Hash -> apr_hash_t */
static int
r2c_hash_i(VALUE key, VALUE value, hash_to_apr_hash_data_t *data)
{
  if (key != Qundef) {
    void *val = data->func(value, data->ctx, data->pool);
    apr_hash_set(data->apr_hash,
                 apr_pstrdup(data->pool, StringValuePtr(key)),
                 APR_HASH_KEY_STRING,
                 val);
  }
  return ST_CONTINUE;
}

static apr_hash_t *
r2c_hash(VALUE hash, r2c_func func, void *ctx, apr_pool_t *pool)
{
  if (NIL_P(hash)) {
    return NULL;
  } else {
    apr_hash_t *apr_hash;
    hash_to_apr_hash_data_t data = {
      NULL,
      func,
      ctx,
      pool
    };

    apr_hash = apr_hash_make(pool);
    data.apr_hash = apr_hash;
    rb_hash_foreach(hash, r2c_hash_i, (VALUE)&data);
    
    return apr_hash;
  }
}


apr_hash_t *
svn_swig_rb_hash_to_apr_hash_string(VALUE hash, apr_pool_t *pool)
{
  return r2c_hash(hash, r2c_string, NULL, pool);
}

apr_hash_t *
svn_swig_rb_hash_to_apr_hash_svn_string(VALUE hash, apr_pool_t *pool)
{
  return r2c_hash(hash, r2c_svn_string, NULL, pool);
}

apr_hash_t *
svn_swig_rb_hash_to_apr_hash_swig_type(VALUE hash, const char *typename, apr_pool_t *pool)
{
  return r2c_hash(hash, r2c_swig_type, (void *)typename, pool);
}

static int
r2c_hash_i_for_revnum(VALUE key, VALUE value, hash_to_apr_hash_data_t *data)
{
  if (key != Qundef) {
    svn_revnum_t *revnum = apr_palloc(data->pool, sizeof(svn_revnum_t));
    *revnum = NUM2INT(value);
    apr_hash_set(data->apr_hash,
                 apr_pstrdup(data->pool, StringValuePtr(key)),
                 APR_HASH_KEY_STRING,
                 (void *)revnum);
  }
  return ST_CONTINUE;
}

apr_hash_t *
svn_swig_rb_hash_to_apr_hash_revnum(VALUE hash, apr_pool_t *pool)
{
  if (NIL_P(hash)) {
    return NULL;
  } else {
    apr_hash_t *apr_hash;
    hash_to_apr_hash_data_t data = {
      NULL,
      NULL,
      NULL,
      pool
    };

    apr_hash = apr_hash_make(pool);
    data.apr_hash = apr_hash;
    rb_hash_foreach(hash, r2c_hash_i_for_revnum, (VALUE)&data);
    
    return apr_hash;
  }
}


/* callback */
typedef struct {
  VALUE pool;
  VALUE receiver;
  ID message;
  VALUE args;
} callback_baton_t;

typedef struct {
  svn_error_t **err;
  VALUE pool;
} callback_rescue_baton_t;

typedef struct {
  callback_baton_t *callback_baton;
  callback_rescue_baton_t *rescue_baton;
} callback_handle_error_baton_t;

static VALUE
callback(VALUE baton)
{
  callback_baton_t *cbb = (callback_baton_t *)baton;
  VALUE result;

  result = rb_apply(cbb->receiver, cbb->message, cbb->args);
  svn_swig_rb_push_pool(cbb->pool);

  return result;
}

static VALUE
callback_rescue(VALUE baton)
{
  callback_rescue_baton_t *rescue_baton = (callback_rescue_baton_t*)baton;

  *(rescue_baton->err) = r2c_svn_err(ruby_errinfo, NULL, NULL);
  svn_swig_rb_push_pool(rescue_baton->pool);
  
  return Qnil;
}

static VALUE
callback_ensure(VALUE pool)
{
  svn_swig_rb_pop_pool(pool);

  return Qnil;
}

static VALUE
invoke_callback(VALUE baton, VALUE pool)
{
  callback_baton_t *cbb = (callback_baton_t *)baton;
  VALUE sub_pool;
  VALUE argv[] = {pool};

  svn_swig_rb_get_pool(1, argv, Qnil, &sub_pool, NULL);
  cbb->pool = sub_pool;
  return rb_ensure(callback, baton, callback_ensure, sub_pool);
}

static VALUE
callback_handle_error(VALUE baton)
{
  callback_handle_error_baton_t *handle_error_baton;
  handle_error_baton = (callback_handle_error_baton_t *)baton;
  
  return rb_rescue2(callback,
                    (VALUE)(handle_error_baton->callback_baton),
                    callback_rescue,
                    (VALUE)(handle_error_baton->rescue_baton),
                    rb_svn_error(),
                    (VALUE)0);
}

static VALUE
invoke_callback_handle_error(VALUE baton, VALUE pool, svn_error_t **err)
{
  callback_baton_t *cbb = (callback_baton_t *)baton;
  callback_handle_error_baton_t handle_error_baton;
  callback_rescue_baton_t rescue_baton;

  rescue_baton.err = err;
  rescue_baton.pool = pool;
  cbb->pool = pool;
  handle_error_baton.callback_baton = cbb;
  handle_error_baton.rescue_baton = &rescue_baton;
  
  return rb_ensure(callback_handle_error, (VALUE)&handle_error_baton,
                   callback_ensure, pool);
}


/* svn_delta_editor_t */
typedef struct {
  VALUE editor;
  VALUE baton;
} item_baton;

static void
add_baton(VALUE editor, VALUE baton)
{
  if (NIL_P((rb_ivar_get(editor, rb_id_baton())))) {
    rb_ivar_set(editor, rb_id_baton(), rb_ary_new());
  }
  
  rb_ary_push(rb_ivar_get(editor, rb_id_baton()), baton);
}

static item_baton *
make_baton(apr_pool_t *pool, VALUE editor, VALUE baton)
{
  item_baton *newb = apr_palloc(pool, sizeof(*newb));

  newb->editor = editor;
  newb->baton = baton;
  add_baton(editor, baton);

  return newb;
}

static VALUE
add_baton_if_delta_editor(VALUE target, VALUE baton)
{
  if (RTEST(rb_obj_is_kind_of(target, svn_swig_rb_svn_delta_editor()))) {
    add_baton(target, baton);
  }

  return Qnil;
}

void
svn_swig_rb_set_baton(VALUE target, VALUE baton)
{
  if (NIL_P(baton)) {
    return;
  }

  if (!RTEST(rb_obj_is_kind_of(target, rb_cArray))) {
    target = rb_ary_new3(1, target);
  }

  rb_iterate(rb_each, target, add_baton_if_delta_editor, baton);
}


static svn_error_t *
delta_editor_set_target_revision(void *edit_baton,
                                 svn_revnum_t target_revision,
                                 apr_pool_t *pool)
{
  item_baton *ib = edit_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_set_target_revision();
  cbb.args = rb_ary_new3(1, INT2NUM(target_revision));
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  return err;
}

static svn_error_t *
delta_editor_open_root(void *edit_baton,
                       svn_revnum_t base_revision,
                       apr_pool_t *dir_pool,
                       void **root_baton)
{
  item_baton *ib = edit_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;
  VALUE result;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_open_root();
  cbb.args = rb_ary_new3(1, INT2NUM(base_revision));
  result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  *root_baton = make_baton(dir_pool, ib->editor, result);
  return err;
}

static svn_error_t *
delta_editor_delete_entry(const char *path,
                          svn_revnum_t revision,
                          void *parent_baton,
                          apr_pool_t *pool)
{
  item_baton *ib = parent_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_delete_entry();
  cbb.args = rb_ary_new3(3, c2r_string2(path), INT2NUM(revision), ib->baton);
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  return err;
}

static svn_error_t *
delta_editor_add_directory(const char *path,
                           void *parent_baton,
                           const char *copyfrom_path,
                           svn_revnum_t copyfrom_revision,
                           apr_pool_t *dir_pool,
                           void **child_baton)
{
  item_baton *ib = parent_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;
  VALUE result;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_add_directory();
  cbb.args = rb_ary_new3(4,
                         c2r_string2(path),
                         ib->baton,
                         c2r_string2(copyfrom_path),
                         INT2NUM(copyfrom_revision));
  result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  *child_baton = make_baton(dir_pool, ib->editor, result);
  return err;
}

static svn_error_t *
delta_editor_open_directory(const char *path,
                            void *parent_baton,
                            svn_revnum_t base_revision,
                            apr_pool_t *dir_pool,
                            void **child_baton)
{
  item_baton *ib = parent_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;
  VALUE result;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_open_directory();
  cbb.args = rb_ary_new3(3,
                         c2r_string2(path),
                         ib->baton,
                         INT2NUM(base_revision));
  result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  *child_baton = make_baton(dir_pool, ib->editor, result);
  return err;
}

static svn_error_t *
delta_editor_change_dir_prop(void *dir_baton,
                             const char *name,
                             const svn_string_t *value,
                             apr_pool_t *pool)
{
  item_baton *ib = dir_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_change_dir_prop();
  cbb.args = rb_ary_new3(3,
                         ib->baton,
                         c2r_string2(name),
                         value ? rb_str_new(value->data, value->len) : Qnil);
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  return err;
}

static svn_error_t *
delta_editor_close_baton(void *baton, ID method_id)
{
  item_baton *ib = baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = method_id;
  cbb.args = rb_ary_new3(1, ib->baton);
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  return err;
}

static svn_error_t *
delta_editor_close_directory(void *dir_baton, apr_pool_t *pool)
{
  return delta_editor_close_baton(dir_baton, rb_id_close_directory());
}

static svn_error_t *
delta_editor_absent_directory(const char *path,
                              void *parent_baton,
                              apr_pool_t *pool)
{
  item_baton *ib = parent_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_absent_directory();
  cbb.args = rb_ary_new3(2, c2r_string2(path), ib->baton);
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  return err;
}

static svn_error_t *
delta_editor_add_file(const char *path,
                      void *parent_baton,
                      const char *copyfrom_path,
                      svn_revnum_t copyfrom_revision,
                      apr_pool_t *file_pool,
                      void **file_baton)
{
  item_baton *ib = parent_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;
  VALUE result;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_add_file();
  cbb.args = rb_ary_new3(4,
                         c2r_string2(path),
                         ib->baton,
                         c2r_string2(copyfrom_path),
                         INT2NUM(copyfrom_revision));
  result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  *file_baton = make_baton(file_pool, ib->editor, result);
  return err;
}

static svn_error_t *
delta_editor_open_file(const char *path,
                       void *parent_baton,
                       svn_revnum_t base_revision,
                       apr_pool_t *file_pool,
                       void **file_baton)
{
  item_baton *ib = parent_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;
  VALUE result;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_open_file();
  cbb.args = rb_ary_new3(3,
                         c2r_string2(path),
                         ib->baton,
                         INT2NUM(base_revision));
  result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  *file_baton = make_baton(file_pool, ib->editor, result);
  return err;
}

static svn_error_t *
delta_editor_window_handler(svn_txdelta_window_t *window, void *baton)
{
  VALUE handler = (VALUE)baton;
  callback_baton_t cbb;
  VALUE result;
  svn_error_t *err = SVN_NO_ERROR;

  cbb.receiver = handler;
  cbb.message = rb_id_call();
  cbb.args = rb_ary_new3(1, c2r_txdelta_window__dup(window));
  result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  return err;
}

static svn_error_t *
delta_editor_apply_textdelta(void *file_baton, 
                             const char *base_checksum,
                             apr_pool_t *pool,
                             svn_txdelta_window_handler_t *handler,
                             void **h_baton)
{
  item_baton *ib = file_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;
  VALUE result;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_apply_textdelta();
  cbb.args = rb_ary_new3(2, ib->baton, c2r_string2(base_checksum));
  result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  if (NIL_P(result)) {
    *handler = svn_delta_noop_window_handler;
    *h_baton = NULL;
  } else {
    *handler = delta_editor_window_handler;
    *h_baton = (void *)result;
  }

  return err;
}

static svn_error_t *
delta_editor_change_file_prop(void *file_baton,
                              const char *name,
                              const svn_string_t *value,
                              apr_pool_t *pool)
{
  item_baton *ib = file_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_change_file_prop();
  cbb.args = rb_ary_new3(3,
                         ib->baton,
                         c2r_string2(name),
                         value ? rb_str_new(value->data, value->len) : Qnil);
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);

  return err;
}

static svn_error_t *
delta_editor_close_file(void *file_baton,
                        const char *text_checksum,
                        apr_pool_t *pool)
{
  item_baton *ib = file_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_close_file();
  cbb.args = rb_ary_new3(2, ib->baton, c2r_string2(text_checksum));
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);

  return err;
}

static svn_error_t *
delta_editor_absent_file(const char *path,
                         void *parent_baton,
                         apr_pool_t *pool)
{
  item_baton *ib = parent_baton;
  svn_error_t *err = SVN_NO_ERROR;
  callback_baton_t cbb;

  cbb.receiver = ib->editor;
  cbb.message = rb_id_absent_file();
  cbb.args = rb_ary_new3(2, c2r_string2(path), ib->baton);
  invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);

  return err;
}

static svn_error_t *
delta_editor_close_edit(void *edit_baton, apr_pool_t *pool)
{
  item_baton *ib = edit_baton;
  svn_error_t *err = delta_editor_close_baton(edit_baton, rb_id_close_edit());
  rb_ary_clear(rb_ivar_get(ib->editor, rb_id_baton()));
  return err;
}

static svn_error_t *
delta_editor_abort_edit(void *edit_baton, apr_pool_t *pool)
{
  item_baton *ib = edit_baton;
  svn_error_t *err = delta_editor_close_baton(edit_baton, rb_id_abort_edit());
  rb_ary_clear(rb_ivar_get(ib->editor, rb_id_baton()));
  return err;
}

void
svn_swig_rb_make_delta_editor(svn_delta_editor_t **editor,
                              void **edit_baton,
                              VALUE rb_editor,
                              apr_pool_t *pool)
{
  svn_delta_editor_t *thunk_editor = svn_delta_default_editor(pool);
  
  thunk_editor->set_target_revision = delta_editor_set_target_revision;
  thunk_editor->open_root = delta_editor_open_root;
  thunk_editor->delete_entry = delta_editor_delete_entry;
  thunk_editor->add_directory = delta_editor_add_directory;
  thunk_editor->open_directory = delta_editor_open_directory;
  thunk_editor->change_dir_prop = delta_editor_change_dir_prop;
  thunk_editor->close_directory = delta_editor_close_directory;
  thunk_editor->absent_directory = delta_editor_absent_directory;
  thunk_editor->add_file = delta_editor_add_file;
  thunk_editor->open_file = delta_editor_open_file;
  thunk_editor->apply_textdelta = delta_editor_apply_textdelta;
  thunk_editor->change_file_prop = delta_editor_change_file_prop;
  thunk_editor->close_file = delta_editor_close_file;
  thunk_editor->absent_file = delta_editor_absent_file;
  thunk_editor->close_edit = delta_editor_close_edit;
  thunk_editor->abort_edit = delta_editor_abort_edit;

  *editor = thunk_editor;
  rb_ivar_set(rb_editor, rb_id_baton(), rb_ary_new());
  *edit_baton = make_baton(pool, rb_editor, Qnil);
}


VALUE
svn_swig_rb_make_baton(VALUE proc, VALUE pool)
{
  if (NIL_P(proc)) {
    return Qnil;
  } else {
    return rb_ary_new3(2, proc, pool);
  }
}

void
svn_swig_rb_from_baton(VALUE baton, VALUE *proc, VALUE *pool)
{
  if (NIL_P(baton)) {
    *proc = Qnil;
    *pool = Qnil;
  } else {
    *proc = rb_ary_entry(baton, 0);
    *pool = rb_ary_entry(baton, 1);
  }
}

svn_error_t *
svn_swig_rb_log_receiver(void *baton,
                         apr_hash_t *changed_paths,
                         svn_revnum_t revision,
                         const char *author,
                         const char *date,
                         const char *message,
                         apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);
  
  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE rb_changed_paths = Qnil;

    if (changed_paths) {
      rb_changed_paths = c2r_hash(changed_paths,
                                  c2r_log_changed_path_dup,
                                  NULL);
    }

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(5,
                           rb_changed_paths,
                           c2r_long(&revision, NULL),
                           c2r_string2(author),
                           c2r_svn_date_string2(date),
                           c2r_string2(message));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  return err;
}


svn_error_t *
svn_swig_rb_repos_authz_func(svn_boolean_t *allowed,
                             svn_fs_root_t *root,
                             const char *path,
                             void *baton,
                             apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  *allowed = TRUE;
  
  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2,
                           c2r_swig_type((void *)root,
                                         (void *)"svn_fs_root_t *"),
                           c2r_string2(path));
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    *allowed = RTEST(result);
  }
  return err;
}

svn_error_t *
svn_swig_rb_repos_authz_callback(svn_repos_authz_access_t required,
                                 svn_boolean_t *allowed,
                                 svn_fs_root_t *root,
                                 const char *path,
                                 void *baton,
                                 apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  *allowed = TRUE;
  
  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(3,
                           INT2NUM(required),
                           c2r_swig_type((void *)root,
                                         (void *)"svn_fs_root_t *"),
                           c2r_string2(path));
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    *allowed = RTEST(result);
  }
  return err;
}

svn_error_t *
svn_swig_rb_get_commit_log_func(const char **log_msg,
                                const char **tmp_file,
                                const apr_array_header_t *commit_items,
                                void *baton,
                                apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  *log_msg = NULL;
  *tmp_file = NULL;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;
    VALUE is_message;
    VALUE value;
    char *ret;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, c2r_commit_item3_array(commit_items));
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    if (!err) {
      char error_message[] =
        "log_msg_func should return an array not '%s': "                \
        "[TRUE_IF_IT_IS_MESSAGE, MESSAGE_OR_FILE_AS_STRING]";

      if (!RTEST(rb_obj_is_kind_of(result, rb_cArray)))
        rb_raise(rb_eTypeError, error_message, r2c_inspect(result));
      is_message = rb_ary_entry(result, 0);
      value = rb_ary_entry(result, 1);

      if (!RTEST(rb_obj_is_kind_of(value, rb_cString)))
        rb_raise(rb_eTypeError, error_message, r2c_inspect(result));
      ret = (char *)r2c_string(value, NULL, pool);
      if (RTEST(is_message)) {
        *log_msg = ret;
      } else {
        *tmp_file = ret;
      }
    }
  }
  return err;
}


void
svn_swig_rb_notify_func2(void *baton,
                         const svn_wc_notify_t *notify,
                         apr_pool_t *pool)
{
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);
  
  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, c2r_wc_notify__dup(notify));
    invoke_callback((VALUE)(&cbb), rb_pool);
  }
}

svn_error_t *
svn_swig_rb_commit_callback(svn_revnum_t new_revision,
                            const char *date,
                            const char *author,
                            void *baton)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(3,
                           INT2NUM(new_revision),
                           c2r_svn_date_string2(date),
                           c2r_string2(author));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_commit_callback2(const svn_commit_info_t *commit_info,
                             void *baton, apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, c2r_commit_info__dup(commit_info));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_cancel_func(void *cancel_baton)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)cancel_baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(0);
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_info_receiver(void *baton,
                          const char *path,
                          const svn_info_t *info,
                          apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2, c2r_string2(path), c2r_info__dup(info));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_boolean_t
svn_swig_rb_config_enumerator(const char *name,
                              const char *value,
                              void *baton,
                              apr_pool_t *pool)
{
  svn_boolean_t result = FALSE;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2, c2r_string2(name), c2r_string2(value));
    result = RTEST(invoke_callback((VALUE)(&cbb), rb_pool));
  }
  
  return result;
}

svn_boolean_t
svn_swig_rb_config_section_enumerator(const char *name,
                                      void *baton,
                                      apr_pool_t *pool)
{
  svn_boolean_t result = FALSE;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, c2r_string2(name));
    result = RTEST(invoke_callback((VALUE)(&cbb), rb_pool));
  }
  
  return result;
}

svn_error_t *
svn_swig_rb_delta_path_driver_cb_func(void **dir_baton,
                                      void *parent_baton,
                                      void *callback_baton,
                                      const char *path,
                                      apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)callback_baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;
    item_baton *ib = (item_baton *)parent_baton;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2, ib->baton, c2r_string2(path));
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
    *dir_baton = make_baton(pool, ib->editor, result);
  }

  return err;
}

svn_error_t *
svn_swig_rb_txdelta_window_handler(svn_txdelta_window_t *window,
                                   void *baton)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, c2r_txdelta_window__dup(window));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

void
svn_swig_rb_fs_warning_callback(void *baton, svn_error_t *err)
{
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, svn_swig_rb_svn_error_to_rb_error(err));
    invoke_callback((VALUE)(&cbb), rb_pool);
  }
}

static apr_status_t
cleanup_fs_warning_callback_baton(void *baton)
{
  rb_holder_pop(rb_svn_fs_warning_callback_baton_holder(), (VALUE)baton);
  return APR_SUCCESS;
}

void
svn_swig_rb_fs_warning_callback_baton_register(VALUE baton, apr_pool_t *pool)
{
  rb_holder_push(rb_svn_fs_warning_callback_baton_holder(), (VALUE)baton);
  apr_pool_cleanup_register(pool, (void *)baton,
                            cleanup_fs_warning_callback_baton,
                            apr_pool_cleanup_null);
}

svn_error_t *
svn_swig_rb_fs_get_locks_callback(void *baton,
                                  svn_lock_t *lock,
                                  apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, c2r_lock__dup(lock));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}


/* svn_ra_callbacks_t */
static svn_error_t *
ra_callbacks_open_tmp_file(apr_file_t **fp,
                           void *callback_baton,
                           apr_pool_t *pool)
{
  VALUE callbacks = (VALUE)callback_baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = callbacks;
    cbb.message = rb_id_open_tmp_file();
    cbb.args = rb_ary_new3(0);
    
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
    *fp = svn_swig_rb_make_file(result, pool);
  }
  
  return err;
}

static svn_error_t *
ra_callbacks_get_wc_prop(void *baton,
                         const char *relpath,
                         const char *name,
                         const svn_string_t **value,
                         apr_pool_t *pool)
{
  VALUE callbacks = (VALUE)baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = callbacks;
    cbb.message = rb_id_get_wc_prop();
    cbb.args = rb_ary_new3(2, c2r_string2(relpath), c2r_string2(name));
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
    if (NIL_P(result)) {
      *value = NULL;
    } else {
      *value = r2c_svn_string(result, NULL, pool);
    }
  }
  
  return err;
}

static svn_error_t *
ra_callbacks_set_wc_prop(void *baton,
                         const char *path,
                         const char *name,
                         const svn_string_t *value,
                         apr_pool_t *pool)
{
  VALUE callbacks = (VALUE)baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;

    cbb.receiver = callbacks;
    cbb.message = rb_id_set_wc_prop();
    cbb.args = rb_ary_new3(3,
                           c2r_string2(path),
                           c2r_string2(name),
                           c2r_svn_string((void *)value, NULL));
    invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  }
  
  return err;
}

static svn_error_t *
ra_callbacks_push_wc_prop(void *baton,
                          const char *path,
                          const char *name,
                          const svn_string_t *value,
                          apr_pool_t *pool)
{
  VALUE callbacks = (VALUE)baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;

    cbb.receiver = callbacks;
    cbb.message = rb_id_push_wc_prop();
    cbb.args = rb_ary_new3(3,
                           c2r_string2(path),
                           c2r_string2(name),
                           c2r_svn_string((void *)value, NULL));
    invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  }
  
  return err;
}

static svn_error_t *
ra_callbacks_invalidate_wc_props(void *baton,
                                 const char *path,
                                 const char *name,
                                 apr_pool_t *pool)
{
  VALUE callbacks = (VALUE)baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;

    cbb.receiver = callbacks;
    cbb.message = rb_id_invalidate_wc_props();
    cbb.args = rb_ary_new3(2, c2r_string2(path), c2r_string2(name));
    invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
  }
  
  return err;
}


static void
ra_callbacks_progress_func(apr_off_t progress,
                           apr_off_t total,
                           void *baton,
                           apr_pool_t *pool)
{
  VALUE callbacks = (VALUE)baton;
  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;

    cbb.receiver = callbacks;
    cbb.message = rb_id_progress_func();
    cbb.args = rb_ary_new3(2, AOFF2NUM(progress), AOFF2NUM(total));
    invoke_callback((VALUE)(&cbb), Qnil);
  }
}

void
svn_swig_rb_setup_ra_callbacks(svn_ra_callbacks2_t **callbacks,
                               void **baton,
                               VALUE rb_callbacks,
                               apr_pool_t *pool)
{
  VALUE rb_auth_baton;

  rb_auth_baton = rb_funcall(rb_callbacks, rb_id_auth_baton(), 0);
  
  *callbacks = apr_pcalloc(pool, sizeof(**callbacks));
  
  (*callbacks)->open_tmp_file = ra_callbacks_open_tmp_file;
  (*callbacks)->auth_baton = r2c_swig_type(rb_auth_baton,
                                           (void *)"svn_auth_baton_t *",
                                           pool);
  (*callbacks)->get_wc_prop = ra_callbacks_get_wc_prop;
  (*callbacks)->set_wc_prop = ra_callbacks_set_wc_prop;
  (*callbacks)->push_wc_prop = ra_callbacks_push_wc_prop;
  (*callbacks)->invalidate_wc_props = ra_callbacks_invalidate_wc_props;
  (*callbacks)->progress_func = ra_callbacks_progress_func;
  (*callbacks)->progress_baton = (void *)rb_callbacks;
}


svn_error_t *
svn_swig_rb_ra_lock_callback(void *baton,
                             const char *path,
                             svn_boolean_t do_lock,
                             const svn_lock_t *lock,
                             svn_error_t *ra_err,
                             apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(4,
                           c2r_string2(path),
                           do_lock ? Qtrue : Qfalse,
                           c2r_lock__dup(lock),
                           ra_err ?
                             svn_swig_rb_svn_error_to_rb_error(ra_err) :
                             Qnil);
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_just_call(void *baton)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(0);
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_ra_file_rev_handler(void *baton,
                                const char *path,
                                svn_revnum_t rev,
                                apr_hash_t *rev_props,
                                svn_txdelta_window_handler_t *delta_handler,
                                void **delta_baton,
                                apr_array_header_t *prop_diffs,
                                apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(4,
                           c2r_string2(path),
                           c2r_long(&rev, NULL),
                           svn_swig_rb_apr_hash_to_hash_svn_string(rev_props),
                           svn_swig_rb_apr_array_to_array_prop(prop_diffs));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_repos_history_func(void *baton,
                               const char *path,
                               svn_revnum_t revision,
                               apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result = Qnil;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2,
                           c2r_string2(path),
                           c2r_long(&revision, NULL));
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    if (!err && SVN_ERR_P(result)) {
      err = r2c_svn_err(result, NULL, NULL);
    }
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_repos_file_rev_handler(void *baton,
                                   const char *path,
                                   svn_revnum_t rev,
                                   apr_hash_t *rev_props,
                                   svn_txdelta_window_handler_t *delta_handler,
                                   void **delta_baton,
                                   apr_array_header_t *prop_diffs,
                                   apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(4,
                           c2r_string2(path),
                           c2r_long(&rev, NULL),
                           svn_swig_rb_apr_hash_to_hash_svn_string(rev_props),
                           svn_swig_rb_apr_array_to_array_prop(prop_diffs));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_error_t *
svn_swig_rb_wc_relocation_validator2(void *baton,
                                     const char *uuid,
                                     const char *url,
                                     svn_boolean_t root,
                                     apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(3,
                           c2r_string2(uuid),
                           c2r_string2(url),
                           root ? Qtrue : Qfalse);
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }

  return err;
}



/* auth provider callbacks */
svn_error_t *
svn_swig_rb_auth_simple_prompt_func(svn_auth_cred_simple_t **cred,
                                    void *baton,
                                    const char *realm,
                                    const char *username,
                                    svn_boolean_t may_save,
                                    apr_pool_t *pool)
{
  svn_auth_cred_simple_t *new_cred = NULL;
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(3,
                           c2r_string2(realm),
                           c2r_string2(username),
                           RTEST(may_save) ? Qtrue : Qfalse);
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    if (!NIL_P(result)) {
      void *result_cred = NULL;
      svn_auth_cred_simple_t *tmp_cred = NULL;
      
      r2c_swig_type2(result, "svn_auth_cred_simple_t *", &result_cred);
      tmp_cred = (svn_auth_cred_simple_t *)result_cred;
      new_cred = apr_pcalloc(pool, sizeof(*new_cred));
      new_cred->username = tmp_cred->username ? \
        apr_pstrdup(pool, tmp_cred->username) : NULL;
      new_cred->password = tmp_cred->password ? \
        apr_pstrdup(pool, tmp_cred->password) : NULL;
      new_cred->may_save = tmp_cred->may_save;
    }
  }
  
  *cred = new_cred;
  return err;
}

svn_error_t *
svn_swig_rb_auth_username_prompt_func(svn_auth_cred_username_t **cred,
                                      void *baton,
                                      const char *realm,
                                      svn_boolean_t may_save,
                                      apr_pool_t *pool)
{
  svn_auth_cred_username_t *new_cred = NULL;
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2,
                           c2r_string2(realm),
                           RTEST(may_save) ? Qtrue : Qfalse);
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    if (!NIL_P(result)) {
      void *result_cred = NULL;
      svn_auth_cred_username_t *tmp_cred = NULL;
      
      r2c_swig_type2(result, "svn_auth_cred_username_t *", &result_cred);
      tmp_cred = (svn_auth_cred_username_t *)result_cred;
      new_cred = apr_pcalloc(pool, sizeof(*new_cred));
      new_cred->username = tmp_cred->username ? \
        apr_pstrdup(pool, tmp_cred->username) : NULL;
      new_cred->may_save = tmp_cred->may_save;
    }
  }
  
  *cred = new_cred;
  return err;
}

svn_error_t *
svn_swig_rb_auth_ssl_server_trust_prompt_func(
  svn_auth_cred_ssl_server_trust_t **cred,
  void *baton,
  const char *realm,
  apr_uint32_t failures,
  const svn_auth_ssl_server_cert_info_t *cert_info,
  svn_boolean_t may_save,
  apr_pool_t *pool)
{
  svn_auth_cred_ssl_server_trust_t *new_cred = NULL;
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(4,
                           c2r_string2(realm),
                           UINT2NUM(failures),
                           c2r_auth_ssl_server_cert_info__dup(cert_info),
                           RTEST(may_save) ? Qtrue : Qfalse);
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    if (!NIL_P(result)) {
      void *result_cred;
      svn_auth_cred_ssl_server_trust_t *tmp_cred = NULL;
      
      r2c_swig_type2(result, "svn_auth_cred_ssl_server_trust_t *",
                     &result_cred);
      tmp_cred = (svn_auth_cred_ssl_server_trust_t *)result_cred;
      new_cred = apr_pcalloc(pool, sizeof(*new_cred));
      *new_cred = *tmp_cred;
    }
  }
  
  *cred = new_cred;
  return err;
}

svn_error_t *
svn_swig_rb_auth_ssl_client_cert_prompt_func(
  svn_auth_cred_ssl_client_cert_t **cred,
  void *baton,
  const char *realm,
  svn_boolean_t may_save,
  apr_pool_t *pool)
{
  svn_auth_cred_ssl_client_cert_t *new_cred = NULL;
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2,
                           c2r_string2(realm),
                           RTEST(may_save) ? Qtrue : Qfalse);
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    if (!NIL_P(result)) {
      void *result_cred = NULL;
      svn_auth_cred_ssl_client_cert_t *tmp_cred = NULL;
      
      r2c_swig_type2(result, "svn_auth_cred_ssl_client_cert_t *",
                     &result_cred);
      tmp_cred = (svn_auth_cred_ssl_client_cert_t *)result_cred;
      new_cred = apr_pcalloc(pool, sizeof(*new_cred));
      new_cred->cert_file = tmp_cred->cert_file ? \
        apr_pstrdup(pool, tmp_cred->cert_file) : NULL;
      new_cred->may_save = tmp_cred->may_save;
    }
  }
  
  *cred = new_cred;
  return err;
}

svn_error_t *
svn_swig_rb_auth_ssl_client_cert_pw_prompt_func(
  svn_auth_cred_ssl_client_cert_pw_t **cred,
  void *baton,
  const char *realm,
  svn_boolean_t may_save,
  apr_pool_t *pool)
{
  svn_auth_cred_ssl_client_cert_pw_t *new_cred = NULL;
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;
    VALUE result;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2,
                           c2r_string2(realm),
                           RTEST(may_save) ? Qtrue : Qfalse);
    result = invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);

    if (!NIL_P(result)) {
      void *result_cred = NULL;
      svn_auth_cred_ssl_client_cert_pw_t *tmp_cred = NULL;
      
      r2c_swig_type2(result, "svn_auth_cred_ssl_client_cert_pw_t *",
                     &result_cred);
      tmp_cred = (svn_auth_cred_ssl_client_cert_pw_t *)result_cred;
      new_cred = apr_pcalloc(pool, sizeof(*new_cred));
      new_cred->password = tmp_cred->password ? \
        apr_pstrdup(pool, tmp_cred->password) : NULL;
      new_cred->may_save = tmp_cred->may_save;
    }
  }
  
  *cred = new_cred;
  return err;
}


apr_file_t *
svn_swig_rb_make_file(VALUE file, apr_pool_t *pool)
{
  apr_file_t *apr_file = NULL;
  
  apr_file_open(&apr_file, StringValuePtr(file),
                APR_CREATE | APR_READ | APR_WRITE,
                APR_OS_DEFAULT,
                pool);
  
  return apr_file;
}


static svn_error_t *
read_handler_rbio(void *baton, char *buffer, apr_size_t *len)
{
  VALUE result;
  VALUE io = (VALUE)baton;
  svn_error_t *err = SVN_NO_ERROR;

  result = rb_funcall(io, rb_id_read(), 1, INT2NUM(*len));
  if (NIL_P(result)) {
    *len = 0;
  } else {
    memcpy(buffer, StringValuePtr(result), RSTRING(result)->len);
    *len = RSTRING(result)->len;
  }

  return err;
}

static svn_error_t *
write_handler_rbio(void *baton, const char *data, apr_size_t *len)
{
  VALUE io = (VALUE)baton;
  svn_error_t *err = SVN_NO_ERROR;

  rb_funcall(io, rb_id_write(), 1, rb_str_new(data, *len));

  return err;
}

svn_stream_t *
svn_swig_rb_make_stream(VALUE io)
{
  svn_stream_t *stream;
  
  if (RTEST(rb_funcall(rb_svn_core_stream(), rb_id_eqq(), 1, io))) {
    svn_stream_t **stream_p;
    stream_p = &stream;
    r2c_swig_type2(io, "svn_stream_t *", (void **)stream_p);
  } else {
    VALUE rb_pool = rb_pool_new(Qnil);
    apr_pool_wrapper_t *pool_wrapper;
    apr_pool_wrapper_t **pool_wrapper_p;
    
    rb_set_pool(io, rb_pool);
    pool_wrapper_p = &pool_wrapper;
    r2c_swig_type2(rb_pool, "apr_pool_wrapper_t *", (void **)pool_wrapper_p);
    stream = svn_stream_create((void *)io, pool_wrapper->pool);
    svn_stream_set_read(stream, read_handler_rbio);
    svn_stream_set_write(stream, write_handler_rbio);
  }
  
  return stream;
}

VALUE
svn_swig_rb_filename_to_temp_file(const char *file_name)
{
  return rb_funcall(rb_svn_util(), rb_id_filename_to_temp_file(),
                    1, rb_str_new2(file_name));
}

void
svn_swig_rb_set_revision(svn_opt_revision_t *rev, VALUE value)
{
  switch (TYPE(value)) {
  case T_NIL:
    rev->kind = svn_opt_revision_unspecified;
    break;
  case T_FIXNUM:
    rev->kind = svn_opt_revision_number;
    rev->value.number = NUM2LONG(value);
    break;
  case T_STRING:
    if (RTEST(rb_reg_match(rb_reg_new("^BASE$",
                                      strlen("^BASE$"),
                                      RE_OPTION_IGNORECASE),
                           value)))
      rev->kind = svn_opt_revision_base;
    else if (RTEST(rb_reg_match(rb_reg_new("^HEAD$",
                                           strlen("^HEAD$"),
                                           RE_OPTION_IGNORECASE),
                                value)))
      rev->kind = svn_opt_revision_head;
    else if (RTEST(rb_reg_match(rb_reg_new("^WORKING$",
                                           strlen("^WORKING$"),
                                           RE_OPTION_IGNORECASE),
                                value)))
      rev->kind = svn_opt_revision_working;
    else if (RTEST(rb_reg_match(rb_reg_new("^COMMITTED$",
                                           strlen("^COMMITTED$"),
                                           RE_OPTION_IGNORECASE),
                                value))) 
      rev->kind = svn_opt_revision_committed;
    else if (RTEST(rb_reg_match(rb_reg_new("^PREV$",
                                           strlen("^PREV$"),
                                           RE_OPTION_IGNORECASE),
                                value))) 
      rev->kind = svn_opt_revision_previous;
    else
      rb_raise(rb_eArgError,
               "invalid value: %s",
               StringValuePtr(value));
    break;
  default:
    if (rb_obj_is_kind_of(value,
                          rb_const_get(rb_cObject, rb_intern("Time")))) {
      rev->kind = svn_opt_revision_date;
      rev->value.date = NUM2LONG(rb_funcall(value, rb_intern("to_i"), 0));
    } else {
      rb_raise(rb_eArgError,
               "invalid type: %s",
               rb_class2name(CLASS_OF(value)));
    }
    break;
  }
}

void
svn_swig_rb_adjust_arg_for_client_ctx_and_pool(int *argc, VALUE **argv)
{
  if (*argc > 1) {
    VALUE last_arg = (*argv)[*argc - 1];
    if (NIL_P(last_arg) || POOL_P(last_arg)) {
      *argv += *argc - 2;
      *argc = 2;
    } else {
      if (CONTEXT_P(last_arg)) {
        *argv += *argc - 1;
        *argc = 1;
      } else {
        *argv += *argc - 2;
        *argc = 2;
      }
    }
  }
}

void
svn_swig_rb_wc_status_func(void *baton,
                           const char *path,
                           svn_wc_status2_t *status)
{
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(2, c2r_string2(path), c2r_wc_status2__dup(status));
    invoke_callback((VALUE)(&cbb), rb_pool);
  }
}

svn_error_t *
svn_swig_rb_client_blame_receiver_func(void *baton,
                                       apr_int64_t line_no,
                                       svn_revnum_t revision,
                                       const char *author,
                                       const char *date,
                                       const char *line,
                                       apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(5,
                           AI642NUM(line_no),
                           INT2NUM(revision),
                           c2r_string2(author),
                           c2r_svn_date_string2(date),
                           c2r_string2(line));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}



/* svn_wc_entry_callbacks_t */
static svn_error_t *
wc_entry_callbacks_found_entry(const char *path,
                               const svn_wc_entry_t *entry,
                               void *walk_baton,
                               apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE callbacks, rb_pool;

  svn_swig_rb_from_baton((VALUE)walk_baton, &callbacks, &rb_pool);;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;

    cbb.receiver = callbacks;
    cbb.message = rb_id_found_entry();
    cbb.args = rb_ary_new3(2,
                           c2r_string2(path),
                           c2r_wc_entry__dup(entry));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }
  
  return err;
}

svn_wc_entry_callbacks_t *
svn_swig_rb_wc_entry_callbacks(void)
{
  static svn_wc_entry_callbacks_t wc_entry_callbacks = {
    wc_entry_callbacks_found_entry
  };

  return &wc_entry_callbacks;
}



/* svn_wc_diff_callbacks2_t */
static svn_error_t *
wc_diff_callbacks_file_changed(svn_wc_adm_access_t *adm_access,
                               svn_wc_notify_state_t *contentstate,
                               svn_wc_notify_state_t *propstate,
                               const char *path,
                               const char *tmpfile1,
                               const char *tmpfile2,
                               svn_revnum_t rev1,
                               svn_revnum_t rev2,
                               const char *mimetype1,
                               const char *mimetype2,
                               const apr_array_header_t *propchanges,
                               apr_hash_t *originalprops,
                               void *diff_baton)
{
  VALUE callbacks = (VALUE)diff_baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result = Qnil;

    cbb.receiver = callbacks;
    cbb.message = rb_id_file_changed();
    cbb.args = rb_ary_new3(10,
                           c2r_swig_type((void *)adm_access,
                                         (void *)"svn_wc_adm_access_t *"),
                           c2r_string2(path),
                           c2r_string2(tmpfile1),
                           c2r_string2(tmpfile2),
                           INT2NUM(rev1),
                           INT2NUM(rev2),
                           c2r_string2(mimetype1),
                           c2r_string2(mimetype2),
                           svn_swig_rb_apr_array_to_array_prop(propchanges),
                           svn_swig_rb_prop_hash_to_hash(originalprops));
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);

    if (contentstate)
      *contentstate = NUM2INT(rb_ary_entry(result, 0));
    if (propstate)
      *propstate = NUM2INT(rb_ary_entry(result, 1));
  }
  
  return err;
}

static svn_error_t *
wc_diff_callbacks_file_added(svn_wc_adm_access_t *adm_access,
                             svn_wc_notify_state_t *contentstate,
                             svn_wc_notify_state_t *propstate,
                             const char *path,
                             const char *tmpfile1,
                             const char *tmpfile2,
                             svn_revnum_t rev1,
                             svn_revnum_t rev2,
                             const char *mimetype1,
                             const char *mimetype2,
                             const apr_array_header_t *propchanges,
                             apr_hash_t *originalprops,
                             void *diff_baton)
{
  VALUE callbacks = (VALUE)diff_baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result = Qnil;

    cbb.receiver = callbacks;
    cbb.message = rb_id_file_added();
    cbb.args = rb_ary_new3(10,
                           c2r_swig_type((void *)adm_access,
                                         (void *)"svn_wc_adm_access_t *"),
                           c2r_string2(path),
                           c2r_string2(tmpfile1),
                           c2r_string2(tmpfile2),
                           INT2NUM(rev1),
                           INT2NUM(rev2),
                           c2r_string2(mimetype1),
                           c2r_string2(mimetype2),
                           svn_swig_rb_apr_array_to_array_prop(propchanges),
                           svn_swig_rb_prop_hash_to_hash(originalprops));
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);

    if (contentstate)
      *contentstate = NUM2INT(rb_ary_entry(result, 0));
    if (propstate)
      *propstate = NUM2INT(rb_ary_entry(result, 1));
  }
  
  return err;
}

static svn_error_t *
wc_diff_callbacks_file_deleted(svn_wc_adm_access_t *adm_access,
                               svn_wc_notify_state_t *state,
                               const char *path,
                               const char *tmpfile1,
                               const char *tmpfile2,
                               const char *mimetype1,
                               const char *mimetype2,
                               apr_hash_t *originalprops,
                               void *diff_baton)
{
  VALUE callbacks = (VALUE)diff_baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result = Qnil;

    cbb.receiver = callbacks;
    cbb.message = rb_id_file_deleted();
    cbb.args = rb_ary_new3(7,
                           c2r_swig_type((void *)adm_access,
                                         (void *)"svn_wc_adm_access_t *"),
                           c2r_string2(path),
                           c2r_string2(tmpfile1),
                           c2r_string2(tmpfile2),
                           c2r_string2(mimetype1),
                           c2r_string2(mimetype2),
                           svn_swig_rb_prop_hash_to_hash(originalprops));
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
    if (state)
      *state = NUM2INT(result);
  }
  
  return err;
}

static svn_error_t *
wc_diff_callbacks_dir_added(svn_wc_adm_access_t *adm_access,
                            svn_wc_notify_state_t *state,
                            const char *path,
                            svn_revnum_t rev,
                            void *diff_baton)
{
  VALUE callbacks = (VALUE)diff_baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result = Qnil;

    cbb.receiver = callbacks;
    cbb.message = rb_id_dir_added();
    cbb.args = rb_ary_new3(3,
                           c2r_swig_type((void *)adm_access,
                                         (void *)"svn_wc_adm_access_t *"),
                           c2r_string2(path),
                           INT2NUM(rev));
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
    if (state)
      *state = NUM2INT(result);
  }
  
  return err;
}

static svn_error_t *
wc_diff_callbacks_dir_deleted(svn_wc_adm_access_t *adm_access,
                              svn_wc_notify_state_t *state,
                              const char *path,
                              void *diff_baton)
{
  VALUE callbacks = (VALUE)diff_baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result = Qnil;

    cbb.receiver = callbacks;
    cbb.message = rb_id_dir_deleted();
    cbb.args = rb_ary_new3(2,
                           c2r_swig_type((void *)adm_access,
                                         (void *)"svn_wc_adm_access_t *"),
                           c2r_string2(path));
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);
    if (state)
      *state = NUM2INT(result);
  }
  
  return err;
}

static svn_error_t *
wc_diff_callbacks_dir_props_changed(svn_wc_adm_access_t *adm_access,
                                    svn_wc_notify_state_t *state,
                                    const char *path,
                                    const apr_array_header_t *propchanges,
                                    apr_hash_t *originalprops,
                                    void *diff_baton)
{
  VALUE callbacks = (VALUE)diff_baton;
  svn_error_t *err = SVN_NO_ERROR;

  if (!NIL_P(callbacks)) {
    callback_baton_t cbb;
    VALUE result = Qnil;

    cbb.receiver = callbacks;
    cbb.message = rb_id_dir_props_changed();
    cbb.args = rb_ary_new3(4,
                           c2r_swig_type((void *)adm_access,
                                         (void *)"svn_wc_adm_access_t *"),
                           c2r_string2(path),
                           svn_swig_rb_apr_array_to_array_prop(propchanges),
                           svn_swig_rb_prop_hash_to_hash(originalprops));
    result = invoke_callback_handle_error((VALUE)(&cbb), Qnil, &err);

    if (state)
      *state = NUM2INT(result);
  }
  
  return err;
}

svn_wc_diff_callbacks2_t *
svn_swig_rb_wc_diff_callbacks2(void)
{
  static svn_wc_diff_callbacks2_t wc_diff_callbacks2 = {
    wc_diff_callbacks_file_changed,
    wc_diff_callbacks_file_added,
    wc_diff_callbacks_file_deleted,
    wc_diff_callbacks_dir_added,
    wc_diff_callbacks_dir_deleted,
    wc_diff_callbacks_dir_props_changed
  };

  return &wc_diff_callbacks2;
}


VALUE
svn_swig_rb_make_txdelta_window_handler_wrapper(VALUE *rb_handler_pool,
                                                apr_pool_t **handler_pool,
                                                svn_txdelta_window_handler_t **handler,
                                                void ***handler_baton)
{
  VALUE obj;

  obj = rb_class_new_instance(0, NULL, rb_cObject);
  svn_swig_rb_get_pool(0, NULL, obj, rb_handler_pool, handler_pool);
  svn_swig_rb_set_pool_for_no_swig_type(obj, *rb_handler_pool);
  *handler = apr_palloc(*handler_pool, sizeof(svn_txdelta_window_handler_t));
  *handler_baton = apr_palloc(*handler_pool, sizeof(void *));

  return obj;
}

VALUE
svn_swig_rb_setup_txdelta_window_handler_wrapper(VALUE obj,
                                                 svn_txdelta_window_handler_t handler,
                                                 void *handler_baton)
{
  rb_ivar_set(obj, rb_id_handler(),
              c2r_swig_type((void *)handler,
                            (void *)"svn_txdelta_window_handler_t"));
  rb_ivar_set(obj, rb_id_handler_baton(),
              c2r_swig_type(handler_baton, (void *)"void *"));
  return obj;
}

svn_error_t *
svn_swig_rb_invoke_txdelta_window_handler_wrapper(VALUE obj,
                                                  svn_txdelta_window_t *window,
                                                  apr_pool_t *pool)
{
  svn_txdelta_window_handler_t handler;
  svn_txdelta_window_handler_t *handler_p;
  void *handler_baton;

  handler_p = &handler;
  r2c_swig_type2(rb_ivar_get(obj, rb_id_handler()),
                 "svn_txdelta_window_handler_t", (void **)handler_p);
  r2c_swig_type2(rb_ivar_get(obj, rb_id_handler_baton()),
                 "void *", &handler_baton);

  return handler(window, handler_baton);
}


VALUE
svn_swig_rb_txdelta_window_t_ops_get(svn_txdelta_window_t *window)
{
  VALUE ops;
  const svn_txdelta_op_t *op;
  int i;

  ops = rb_ary_new2(window->num_ops);

  for (i = 0; i < window->num_ops; i++) {
    op = window->ops + i;
    rb_ary_push(ops, c2r_swig_type((void *)op, (void *)"svn_txdelta_op_t *"));
  }

  return ops;
}


svn_error_t *
svn_swig_rb_client_diff_summarize_func(const svn_client_diff_summarize_t *diff,
                                       void *baton,
                                       apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(1, c2r_client_diff_summarize__dup(diff));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }

  return err;
}

svn_error_t *
svn_swig_rb_client_list_func(void *baton,
                             const char *path,
                             const svn_dirent_t *dirent,
                             const svn_lock_t *lock,
                             const char *abs_path,
                             apr_pool_t *pool)
{
  svn_error_t *err = SVN_NO_ERROR;
  VALUE proc, rb_pool;

  svn_swig_rb_from_baton((VALUE)baton, &proc, &rb_pool);

  if (!NIL_P(proc)) {
    callback_baton_t cbb;

    cbb.receiver = proc;
    cbb.message = rb_id_call();
    cbb.args = rb_ary_new3(4,
                           c2r_string2(path),
                           c2r_dirent__dup(dirent),
                           c2r_lock__dup(lock),
                           c2r_string2(abs_path));
    invoke_callback_handle_error((VALUE)(&cbb), rb_pool, &err);
  }

  return err;
}

