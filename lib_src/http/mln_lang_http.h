
/*
 * Copyright (C) Niklaus F.Schen.
 */
#ifndef __MLN_LANG_HTTP_H
#define __MLN_LANG_HTTP_H

#include "mln_lang.h"

#if defined(MLN_TLS)
/* Shared with lib_src/tls so that an http.https_request() call can
 * reuse a SSL_CTX configured by tls.conf_new() without each module
 * having to walk a separate rbtree. */
#include "mln_connection.h"
#endif

#endif

