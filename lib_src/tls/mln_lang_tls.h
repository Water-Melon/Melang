
/*
 * Copyright (C) Niklaus F.Schen.
 */
#ifndef __MLN_LANG_TLS_H
#define __MLN_LANG_TLS_H

#include "mln_lang.h"

#if defined(MLN_TLS)

#include "mln_connection.h"

/*
 * mln_lang_tls_t wraps either:
 *   - a plain TCP listening socket (is_listen=1, conn.ssl=NULL), or
 *   - a TLS-capable connection (is_listen=0, conn.ssl=non-NULL after
 *     mln_tcp_conn_tls_init).
 *
 * Lifetime: kept alive in the per-lang "tls" rbtree until the script
 * calls tls.close() (or the coroutine that holds it terminates).  When
 * an event handler is pending the connection is also linked into the
 * per-ctx "tls" chain so that early coroutine teardown can disarm the
 * event and drop the suspend reference safely.
 */
/*
 * The per-accept "request bag" is a small heap allocation that carries
 * (listen lt, conf id) across the suspend boundary.  It is opaque to
 * the rest of the module -- only the accept paths know its layout --
 * but the listen lt holds a back-pointer so that a coroutine teardown
 * which runs before the accept completes can free the bag too.
 */
struct mln_lang_tls_accept_req_s;

typedef struct mln_lang_tls_s {
    mln_lang_t            *lang;
    mln_lang_ctx_t        *ctx;
    mln_tcp_conn_t         conn;
    char                   ip[128];
    mln_u16_t              port;
    mln_u16_t              is_listen:1;
    mln_u16_t              send_closed:1;
    mln_u16_t              recv_closed:1;
    mln_u16_t              sending:1;
    mln_u16_t              recving:1;
    mln_u16_t              shutting:1;
    mln_u16_t              retry:10;
    mln_s32_t              timeout;
    mln_s32_t              connect_timeout;
    /* Non-NULL only while an accept is pending on this listen lt;
     * tls_free() inspects this field and frees the bag if the
     * coroutine vanishes before the accept handler fires. */
    struct mln_lang_tls_accept_req_s *accept_req;
    struct mln_lang_tls_s *prev;
    struct mln_lang_tls_s *next;
} mln_lang_tls_t;

typedef struct mln_lang_ctx_tls_s {
    mln_lang_ctx_t        *ctx;
    mln_lang_tls_t        *head;
    mln_lang_tls_t        *tail;
} mln_lang_ctx_tls_t;

/*
 * Configuration handle: scripts hold an integer id and the rbtree maps
 * the id back to the underlying SSL_CTX wrapper.  Ids are issued from
 * a monotonically increasing counter; freed ids are not reused so that
 * a stale script-side handle never aliases a different conf.
 */
typedef struct mln_lang_tls_conf_s {
    mln_s64_t              id;
    mln_tcp_tls_conf_t    *conf;
} mln_lang_tls_conf_t;

#define MLN_LANG_TLS_CONNECT_RETRY 64

#endif /* MLN_TLS */

#endif
