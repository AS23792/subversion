/**
 * @copyright
 * ====================================================================
 * Copyright (c) 2000-2003 CollabNet.  All rights reserved.
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
 * @endcopyright
 *
 * @file svn_md5.h
 * @brief Code for converting and comparing MD5 checksums.
 */

#include <apr_pools.h>
#include <apr_md5.h>
#include "svn_error.h"
#include "svn_pools.h"



/* The MD5 digest for the empty string. */
extern const char svn_md5_empty_string_digest[];


/** Return the hex representation of @a digest, which must be
 * MD5_DIGESTSIZE bytes long.  Allocate the string in @a pool.
 */
const char *svn_md5_digest_to_cstring (unsigned char digest[],
                                       apr_pool_t *pool);


/* Compare digests @a d1 and @a d2, each MD5_DIGESTSIZE bytes long.
 * If neither is all zeros, and they do not match, then return false;
 * else return true.
 */
svn_boolean_t svn_md5_digests_match (unsigned const char d1[],
                                     unsigned const char d2[]);
