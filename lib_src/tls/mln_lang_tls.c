
/*
 * Copyright (C) Niklaus F.Schen.
 *
 * Melang TLS module.  Wraps the TLS support that Melon's mln_tcp_conn
 * gained in commit 8e6fede so scripts can do non-blocking TLS exactly
 * like they do plain TCP through the `network` module.
 *
 * The whole module is gated by MLN_TLS so a build of Melon without
 * --enable-tls produces a stub that fails Import('tls') at load time
 * rather than crashing on the first SSL call.
 */
#include "mln_lang_tls.h"
#include "mln_log.h"
#include "mln_conf.h"

#if !defined(MLN_TLS)

mln_lang_var_t *init(mln_lang_ctx_t *ctx, mln_conf_t *cf)
{
    (void)cf;
    mln_lang_errmsg(ctx, "TLS support not compiled in. Rebuild Melon with --enable-tls.");
    return NULL;
}

#else /* MLN_TLS */

#if defined(WIN32)
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

/* ---- forward declarations ------------------------------------------------ */

MLN_CHAIN_FUNC_DECLARE(static inline, mln_lang_tls, mln_lang_tls_t,);
MLN_CHAIN_FUNC_DEFINE (static inline, mln_lang_tls, mln_lang_tls_t, prev, next);

static int mln_lang_tls(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_resource_register(mln_lang_ctx_t *ctx);

/* resource helpers */
static int  mln_lang_tls_cmp(const mln_lang_tls_t *a, const mln_lang_tls_t *b);
static void mln_lang_tls_free(mln_lang_tls_t *lt);
static mln_lang_tls_t *mln_lang_tls_new_listen(mln_lang_t *lang, int fd,
                                               const char *ip, mln_u16_t port);
static mln_lang_tls_t *mln_lang_tls_new_conn(mln_lang_t *lang, int fd,
                                             mln_tcp_tls_conf_t *conf,
                                             const char *ip, mln_u16_t port);
static int  mln_lang_tls_resource_add(mln_lang_t *lang, mln_lang_tls_t *lt);
static mln_lang_tls_t *mln_lang_tls_resource_fetch(mln_lang_t *lang, int fd);
static void mln_lang_tls_resource_remove(mln_lang_t *lang, int fd);
static int  mln_lang_tls_conf_cmp(const mln_lang_tls_conf_t *a, const mln_lang_tls_conf_t *b);
static void mln_lang_tls_conf_node_free(mln_lang_tls_conf_t *node);
static mln_lang_tls_conf_t *mln_lang_tls_conf_lookup(mln_lang_t *lang, mln_s64_t id);

static mln_lang_ctx_tls_t *mln_lang_ctx_tls_new(mln_lang_ctx_t *ctx);
static void mln_lang_ctx_tls_free(mln_lang_ctx_tls_t *lct);
static void mln_lang_ctx_tls_resource_add(mln_lang_ctx_t *ctx, mln_lang_tls_t *lt);
static void mln_lang_ctx_tls_resource_remove(mln_lang_tls_t *lt);

/* utility */
static int  mln_lang_tls_get_addr(const void *in_addr, char *ip, mln_u16_t *port);
static int  mln_lang_tls_optional_string(mln_lang_symbol_node_t *sym, mln_string_t **out);

/* registrations */
static int mln_lang_tls_conf_new_reg (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_conf_free_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_listen_reg   (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_accept_reg   (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_connect_reg  (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_set_sni_reg  (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_set_verify_host_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_handshake_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_send_reg     (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_recv_reg     (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_tls_close_reg    (mln_lang_ctx_t *ctx, mln_lang_object_t *obj);

/* process functions */
static mln_lang_var_t *mln_lang_tls_conf_new_process    (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_conf_free_process   (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_listen_process      (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_accept_process      (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_connect_process     (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_set_sni_process     (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_set_verify_host_process(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_handshake_process   (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_send_process        (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_recv_process        (mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_tls_close_process       (mln_lang_ctx_t *ctx);

/* event handlers */
static void mln_lang_tls_accept_handler         (mln_event_t *ev, int fd, void *data);
static void mln_lang_tls_accept_timeout_handler (mln_event_t *ev, int fd, void *data);
static void mln_lang_tls_connect_handler        (mln_event_t *ev, int fd, void *data);
static void mln_lang_tls_connect_timeout_handler(mln_event_t *ev, int fd, void *data);
static void mln_lang_tls_handshake_handler      (mln_event_t *ev, int fd, void *data);
static void mln_lang_tls_recv_handler           (mln_event_t *ev, int fd, void *data);
static void mln_lang_tls_send_handler           (mln_event_t *ev, int fd, void *data);
static void mln_lang_tls_timeout_handler        (mln_event_t *ev, int fd, void *data);

/* per-want event rearming.  Returns 0 on success, -1 on failure. */
static int  mln_lang_tls_rearm(mln_lang_tls_t *lt, mln_event_t *ev,
                               mln_u32_t fallback_flag,
                               int timeout,
                               ev_fd_handler handler);

/*
 * Per-accept request bag: an accept() call needs both the listen
 * mln_lang_tls_t and the chosen conf id carried across the suspend
 * boundary.  We allocate one on registration and free it on every
 * termination edge of the handlers (success, error, spurious wakeup
 * limit, timeout) and also from tls_free if the coroutine terminates
 * before the accept completes.  See the typedef in mln_lang_tls.h.
 */
struct mln_lang_tls_accept_req_s {
    mln_lang_tls_t *lt;
    mln_s64_t       conf_id;
};
typedef struct mln_lang_tls_accept_req_s mln_lang_tls_accept_req_t;
/* legacy spelling kept to minimize diffs in callers */
#define mln_lang_tls_accept_req mln_lang_tls_accept_req_s

/* ---- init ---------------------------------------------------------------- */

mln_lang_var_t *init(mln_lang_ctx_t *ctx, mln_conf_t *cf)
{
    mln_log_init(cf);

    mln_lang_var_t *obj = mln_lang_var_create_obj(ctx, NULL, NULL);
    if (obj == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (mln_lang_tls(ctx, mln_lang_var_val_get(obj)->data.obj) < 0) {
        mln_lang_var_free(obj);
        return NULL;
    }
    return obj;
}

/*
 * destroy is invoked at lang teardown; cancel per-lang resources only
 * after their rbtrees have already drained because every live tls_t
 * holds an fd whose close path is wired through mln_lang_tls_free.
 */
void destroy(mln_lang_t *lang)
{
    mln_rbtree_t *tls_set, *conf_set;
    if ((tls_set = mln_lang_resource_fetch(lang, "tls")) != NULL) {
        if (!mln_rbtree_node_num(tls_set))
            mln_lang_resource_cancel(lang, "tls");
    }
    if ((conf_set = mln_lang_resource_fetch(lang, "tls_conf")) != NULL) {
        if (!mln_rbtree_node_num(conf_set))
            mln_lang_resource_cancel(lang, "tls_conf");
    }
    mln_lang_resource_cancel(lang, "tls_conf_next_id");
}

static int mln_lang_tls(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    if (mln_lang_tls_resource_register(ctx) < 0) return -1;
    if (mln_lang_tls_conf_new_reg(ctx, obj)  < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_conf_free_reg(ctx, obj) < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_listen_reg(ctx, obj)    < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_accept_reg(ctx, obj)    < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_connect_reg(ctx, obj)   < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_set_sni_reg(ctx, obj)   < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_set_verify_host_reg(ctx, obj) < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_handshake_reg(ctx, obj) < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_send_reg(ctx, obj)      < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_recv_reg(ctx, obj)      < 0) { destroy(ctx->lang); return -1; }
    if (mln_lang_tls_close_reg(ctx, obj)     < 0) { destroy(ctx->lang); return -1; }
    return 0;
}

static int mln_lang_tls_resource_register(mln_lang_ctx_t *ctx)
{
    mln_rbtree_t *tls_set, *conf_set;
    mln_s64_t    *id_counter;

    if ((tls_set = mln_lang_resource_fetch(ctx->lang, "tls")) == NULL) {
#if !defined(WIN32)
        signal(SIGPIPE, SIG_IGN);
#endif
        struct mln_rbtree_attr rbattr;
        rbattr.pool       = ctx->lang->pool;
        rbattr.pool_alloc = (rbtree_pool_alloc_handler)mln_alloc_m;
        rbattr.pool_free  = (rbtree_pool_free_handler)mln_alloc_free;
        rbattr.cmp        = (rbtree_cmp)mln_lang_tls_cmp;
        rbattr.data_free  = (rbtree_free_data)mln_lang_tls_free;
        if ((tls_set = mln_rbtree_new(&rbattr)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
        if (mln_lang_resource_register(ctx->lang, "tls", tls_set,
                                       (mln_lang_resource_free)mln_rbtree_free) < 0) {
            mln_rbtree_free(tls_set);
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
    }

    if ((conf_set = mln_lang_resource_fetch(ctx->lang, "tls_conf")) == NULL) {
        struct mln_rbtree_attr rbattr;
        rbattr.pool       = ctx->lang->pool;
        rbattr.pool_alloc = (rbtree_pool_alloc_handler)mln_alloc_m;
        rbattr.pool_free  = (rbtree_pool_free_handler)mln_alloc_free;
        rbattr.cmp        = (rbtree_cmp)mln_lang_tls_conf_cmp;
        rbattr.data_free  = (rbtree_free_data)mln_lang_tls_conf_node_free;
        if ((conf_set = mln_rbtree_new(&rbattr)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
        if (mln_lang_resource_register(ctx->lang, "tls_conf", conf_set,
                                       (mln_lang_resource_free)mln_rbtree_free) < 0) {
            mln_rbtree_free(conf_set);
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
    }

    if ((id_counter = mln_lang_resource_fetch(ctx->lang, "tls_conf_next_id")) == NULL) {
        if ((id_counter = (mln_s64_t *)malloc(sizeof(mln_s64_t))) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
        *id_counter = 1;
        if (mln_lang_resource_register(ctx->lang, "tls_conf_next_id",
                                       id_counter,
                                       (mln_lang_resource_free)free) < 0) {
            free(id_counter);
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
    }

    mln_lang_ctx_tls_t *lct;
    if ((lct = mln_lang_ctx_resource_fetch(ctx, "tls")) == NULL) {
        if ((lct = mln_lang_ctx_tls_new(ctx)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
        if (mln_lang_ctx_resource_register(ctx, "tls", lct,
                                           (mln_lang_resource_free)mln_lang_ctx_tls_free) < 0) {
            mln_lang_ctx_tls_free(lct);
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
    }
    return 0;
}

/* ---- low-level state plumbing ------------------------------------------- */

static int mln_lang_tls_cmp(const mln_lang_tls_t *a, const mln_lang_tls_t *b)
{
    return mln_tcp_conn_fd_get((mln_tcp_conn_t *)&a->conn) -
           mln_tcp_conn_fd_get((mln_tcp_conn_t *)&b->conn);
}

static int mln_lang_tls_conf_cmp(const mln_lang_tls_conf_t *a,
                                 const mln_lang_tls_conf_t *b)
{
    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;
    return 0;
}

static void mln_lang_tls_conf_node_free(mln_lang_tls_conf_t *node)
{
    if (node == NULL) return;
    if (node->conf != NULL) mln_tcp_tls_conf_free(node->conf);
    free(node);
}

static mln_lang_tls_conf_t *mln_lang_tls_conf_lookup(mln_lang_t *lang, mln_s64_t id)
{
    mln_lang_tls_conf_t tmp;
    mln_rbtree_node_t  *rn;
    mln_rbtree_t       *conf_set = mln_lang_resource_fetch(lang, "tls_conf");
    if (conf_set == NULL) return NULL;
    tmp.id   = id;
    tmp.conf = NULL;
    rn = mln_rbtree_search(conf_set, &tmp);
    if (mln_rbtree_null(rn, conf_set)) return NULL;
    return (mln_lang_tls_conf_t *)mln_rbtree_node_data_get(rn);
}

static mln_lang_tls_t *mln_lang_tls_new_listen(mln_lang_t *lang, int fd,
                                               const char *ip, mln_u16_t port)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)malloc(sizeof(*lt));
    if (lt == NULL) return NULL;
    memset(lt, 0, sizeof(*lt));
    lt->lang = lang;
    lt->ctx  = NULL;
    if (mln_tcp_conn_init(&lt->conn, fd) < 0) { free(lt); return NULL; }
    int n = snprintf(lt->ip, sizeof(lt->ip), "%s", ip ? ip : "");
    if (n < 0) n = 0;
    if ((size_t)n >= sizeof(lt->ip)) n = (int)sizeof(lt->ip) - 1;
    lt->ip[n] = '\0';
    lt->port = port;
    lt->is_listen = 1;
    lt->timeout = M_EV_UNLIMITED;
    lt->connect_timeout = M_EV_UNLIMITED;
    return lt;
}

static mln_lang_tls_t *mln_lang_tls_new_conn(mln_lang_t *lang, int fd,
                                             mln_tcp_tls_conf_t *conf,
                                             const char *ip, mln_u16_t port)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)malloc(sizeof(*lt));
    if (lt == NULL) return NULL;
    memset(lt, 0, sizeof(*lt));
    lt->lang = lang;
    lt->ctx  = NULL;
    if (mln_tcp_conn_tls_init(&lt->conn, fd, conf) < 0) { free(lt); return NULL; }
    /* Every Melang TLS socket is driven from the event loop, so the
     * socket is always non-blocking; mirror that into the conn struct
     * so mln_tcp_conn_tls_handshake returns M_C_NOTYET instead of
     * spinning on EAGAIN inside drain_socket. */
    mln_tcp_conn_set_nonblock(&lt->conn, 1);
    int n = snprintf(lt->ip, sizeof(lt->ip), "%s", ip ? ip : "");
    if (n < 0) n = 0;
    if ((size_t)n >= sizeof(lt->ip)) n = (int)sizeof(lt->ip) - 1;
    lt->ip[n] = '\0';
    lt->port = port;
    lt->is_listen = 0;
    lt->timeout = M_EV_UNLIMITED;
    lt->connect_timeout = M_EV_UNLIMITED;
    return lt;
}

static void mln_lang_tls_free(mln_lang_tls_t *lt)
{
    if (lt == NULL) return;
    int fd = mln_tcp_conn_fd_get(&lt->conn);

    /* If a coroutine is currently suspended on this connection we must
     * resume it; otherwise the script side would hang forever on a
     * connection that has just disappeared underneath it. */
    if (lt->ctx != NULL) {
        mln_lang_ctx_tls_t *lct = mln_lang_ctx_resource_fetch(lt->ctx, "tls");
        if (lct != NULL) {
            mln_lang_tls_chain_del(&lct->head, &lct->tail, lt);
            mln_lang_ctx_continue(lt->ctx);
            lt->ctx = NULL;
            lt->sending = lt->recving = lt->shutting = 0;
        }
    }
    if (fd >= 0) {
        mln_event_fd_set(lt->lang->ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
        mln_socket_close(fd);
    }
    mln_tcp_conn_destroy(&lt->conn);
    /* If an accept was pending the request bag is owned by the event
     * registration we just cleared; reclaim it here so it does not
     * leak when the listen lt is dropped before its handler fires. */
    if (lt->accept_req != NULL) {
        free(lt->accept_req);
        lt->accept_req = NULL;
    }
    free(lt);
}

static int mln_lang_tls_resource_add(mln_lang_t *lang, mln_lang_tls_t *lt)
{
    mln_rbtree_node_t *rn;
    mln_rbtree_t *tls_set = mln_lang_resource_fetch(lang, "tls");
    if (tls_set == NULL) return -1;
    if ((rn = mln_rbtree_node_new(tls_set, lt)) == NULL) return -1;
    mln_rbtree_insert(tls_set, rn);
    return 0;
}

static mln_lang_tls_t *mln_lang_tls_resource_fetch(mln_lang_t *lang, int fd)
{
    mln_lang_tls_t tmp;
    mln_rbtree_node_t *rn;
    mln_rbtree_t *tls_set = mln_lang_resource_fetch(lang, "tls");
    if (tls_set == NULL) return NULL;
    memset(&tmp, 0, sizeof(tmp));
    mln_tcp_conn_fd_set(&tmp.conn, fd);
    rn = mln_rbtree_search(tls_set, &tmp);
    if (mln_rbtree_null(rn, tls_set)) return NULL;
    return (mln_lang_tls_t *)mln_rbtree_node_data_get(rn);
}

static void mln_lang_tls_resource_remove(mln_lang_t *lang, int fd)
{
    mln_lang_tls_t tmp;
    mln_rbtree_node_t *rn;
    mln_rbtree_t *tls_set = mln_lang_resource_fetch(lang, "tls");
    if (tls_set == NULL) return;
    memset(&tmp, 0, sizeof(tmp));
    mln_tcp_conn_fd_set(&tmp.conn, fd);
    rn = mln_rbtree_search(tls_set, &tmp);
    if (mln_rbtree_null(rn, tls_set)) return;
    mln_rbtree_delete(tls_set, rn);
    mln_rbtree_node_free(tls_set, rn);
}

static mln_lang_ctx_tls_t *mln_lang_ctx_tls_new(mln_lang_ctx_t *ctx)
{
    mln_lang_ctx_tls_t *lct = (mln_lang_ctx_tls_t *)mln_alloc_m(ctx->pool, sizeof(*lct));
    if (lct == NULL) return NULL;
    lct->ctx  = ctx;
    lct->head = lct->tail = NULL;
    return lct;
}

static void mln_lang_ctx_tls_free(mln_lang_ctx_tls_t *lct)
{
    if (lct == NULL) return;
    mln_lang_tls_t *lt;
    while ((lt = lct->head) != NULL) {
        mln_lang_tls_chain_del(&lct->head, &lct->tail, lt);
        mln_event_fd_set(lt->lang->ev, mln_tcp_conn_fd_get(&lt->conn),
                         M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
        /* If this coroutine vanished mid-accept the request bag is no
         * longer reachable through any event registration; reclaim it
         * now so a subsequent tls.accept on the same listen lt does
         * not overwrite (and leak) the stale bag pointer. */
        if (lt->accept_req != NULL) {
            free(lt->accept_req);
            lt->accept_req = NULL;
        }
        lt->ctx = NULL;
        lt->sending = lt->recving = lt->shutting = 0;
    }
    mln_alloc_free(lct);
}

static void mln_lang_ctx_tls_resource_add(mln_lang_ctx_t *ctx, mln_lang_tls_t *lt)
{
    mln_lang_ctx_tls_t *lct = mln_lang_ctx_resource_fetch(ctx, "tls");
    ASSERT(lct != NULL);
    mln_lang_tls_chain_add(&lct->head, &lct->tail, lt);
    lt->ctx = ctx;
}

static void mln_lang_ctx_tls_resource_remove(mln_lang_tls_t *lt)
{
    if (lt->ctx == NULL) return;
    mln_lang_ctx_tls_t *lct = mln_lang_ctx_resource_fetch(lt->ctx, "tls");
    if (lct == NULL) {
        lt->ctx = NULL;
        return;
    }
    mln_lang_tls_chain_del(&lct->head, &lct->tail, lt);
    lt->ctx = NULL;
}

/* ---- helpers ------------------------------------------------------------- */

static int mln_lang_tls_get_addr(const void *in_addr, char *ip, mln_u16_t *port)
{
    const struct sockaddr *saddr = (const struct sockaddr *)in_addr;
    const void *numeric_addr = NULL;
    char addr_buf[128] = {0};

    if (saddr->sa_family == AF_INET) {
        const struct sockaddr_in *a = (const struct sockaddr_in *)in_addr;
        numeric_addr = &a->sin_addr;
        *port = ntohs(a->sin_port);
    } else if (saddr->sa_family == AF_INET6) {
        const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)in_addr;
        numeric_addr = &a->sin6_addr;
        *port = ntohs(a->sin6_port);
    } else {
        return -1;
    }
    if (inet_ntop(saddr->sa_family, numeric_addr, addr_buf, sizeof(addr_buf)) == NULL)
        return -1;
    memcpy(ip, addr_buf, sizeof(addr_buf));
    return 0;
}

static int mln_lang_tls_optional_string(mln_lang_symbol_node_t *sym, mln_string_t **out)
{
    mln_u32_t t = mln_lang_var_val_type_get(sym->data.var);
    if (t == M_LANG_VAL_TYPE_NIL) { *out = NULL; return 0; }
    if (t == M_LANG_VAL_TYPE_STRING) {
        *out = sym->data.var->val->data.s;
        return 0;
    }
    return -1;
}

/* Generic function registration helper. */
typedef struct {
    const char *name;
    mln_lang_internal handler;
    const char *args[6];
    int         nargs;
} mln_lang_tls_funcdef_t;

static int
mln_lang_tls_register_func(mln_lang_ctx_t *ctx, mln_lang_object_t *obj,
                           const mln_lang_tls_funcdef_t *def)
{
    mln_lang_func_detail_t *func;
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_string_t name_s;
    mln_string_nset(&name_s, (char *)def->name, strlen(def->name));

    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL,
                                         def->handler, NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    for (int i = 0; i < def->nargs; ++i) {
        mln_string_t arg_s;
        mln_string_nset(&arg_s, (char *)def->args[i], strlen(def->args[i]));
        if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            mln_lang_func_detail_free(func);
            return -1;
        }
        if ((var = mln_lang_var_new(ctx, &arg_s, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            mln_lang_val_free(val);
            mln_lang_func_detail_free(func);
            return -1;
        }
        mln_lang_func_detail_arg_append(func, var);
    }
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_FUNC, func)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &name_s, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        return -1;
    }
    if (mln_lang_set_member_add(ctx->pool, obj->members, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

/* ---- conf_new (server/client config builder) ---------------------------- */

static int mln_lang_tls_conf_new_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "conf_new", mln_lang_tls_conf_new_process,
        { "role", "cert", "key", "ca", "ciphers", "verify" }, 6,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_conf_new_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_role    = mln_string("role");
    mln_string_t v_cert    = mln_string("cert");
    mln_string_t v_key     = mln_string("key");
    mln_string_t v_ca      = mln_string("ca");
    mln_string_t v_ciphers = mln_string("ciphers");
    mln_string_t v_verify  = mln_string("verify");
    mln_string_t  s_server = mln_string("server");
    mln_string_t  s_client = mln_string("client");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    mln_string_t *role_s = NULL, *cert = NULL, *key = NULL, *ca = NULL, *ciphers = NULL;
    mln_u32_t role;
    mln_u32_t verify = 0;
    mln_rbtree_t *conf_set;
    mln_rbtree_node_t *rn;
    mln_lang_tls_conf_t *node;
    mln_s64_t *id_counter;
    mln_tcp_tls_conf_t *conf;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_role, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'role' must be 'server' or 'client'."); return NULL; }
    role_s = sym->data.var->val->data.s;
    if      (!mln_string_strcmp(role_s, &s_server)) role = M_TLS_SERVER;
    else if (!mln_string_strcmp(role_s, &s_client)) role = M_TLS_CLIENT;
    else { mln_lang_errmsg(ctx, "Argument 'role' must be 'server' or 'client'."); return NULL; }

    if ((sym = mln_lang_symbol_node_search(ctx, &v_cert, 1)) == NULL ||
        mln_lang_tls_optional_string(sym, &cert) < 0)
    { mln_lang_errmsg(ctx, "Argument 'cert' must be a string or nil."); return NULL; }
    if ((sym = mln_lang_symbol_node_search(ctx, &v_key,  1)) == NULL ||
        mln_lang_tls_optional_string(sym, &key)  < 0)
    { mln_lang_errmsg(ctx, "Argument 'key' must be a string or nil."); return NULL; }
    if ((sym = mln_lang_symbol_node_search(ctx, &v_ca,   1)) == NULL ||
        mln_lang_tls_optional_string(sym, &ca)   < 0)
    { mln_lang_errmsg(ctx, "Argument 'ca' must be a string or nil."); return NULL; }
    if ((sym = mln_lang_symbol_node_search(ctx, &v_ciphers, 1)) == NULL ||
        mln_lang_tls_optional_string(sym, &ciphers) < 0)
    { mln_lang_errmsg(ctx, "Argument 'ciphers' must be a string or nil."); return NULL; }

    if ((sym = mln_lang_symbol_node_search(ctx, &v_verify, 1)) == NULL) {
        mln_lang_errmsg(ctx, "Argument 'verify' missing."); return NULL;
    } else {
        mln_u32_t t = mln_lang_var_val_type_get(sym->data.var);
        if      (t == M_LANG_VAL_TYPE_NIL)  verify = 0;
        else if (t == M_LANG_VAL_TYPE_BOOL) verify = sym->data.var->val->data.b ? 1 : 0;
        else if (t == M_LANG_VAL_TYPE_INT)  verify = sym->data.var->val->data.i ? 1 : 0;
        else { mln_lang_errmsg(ctx, "Argument 'verify' must be bool, int or nil."); return NULL; }
    }

    conf = mln_tcp_tls_conf_new(role, cert, key, ca, ciphers, M_TLS_VDEFAULT, verify);
    if (conf == NULL) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }

    conf_set   = mln_lang_resource_fetch(ctx->lang, "tls_conf");
    id_counter = mln_lang_resource_fetch(ctx->lang, "tls_conf_next_id");
    if (conf_set == NULL || id_counter == NULL) {
        mln_tcp_tls_conf_free(conf);
        mln_lang_errmsg(ctx, "TLS resources unavailable.");
        return NULL;
    }
    node = (mln_lang_tls_conf_t *)malloc(sizeof(*node));
    if (node == NULL) {
        mln_tcp_tls_conf_free(conf);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    node->id   = (*id_counter)++;
    node->conf = conf;
    if ((rn = mln_rbtree_node_new(conf_set, node)) == NULL) {
        free(node);
        mln_tcp_tls_conf_free(conf);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    mln_rbtree_insert(conf_set, rn);

    if ((ret_var = mln_lang_var_create_int(ctx, node->id, NULL)) == NULL) {
        mln_rbtree_delete(conf_set, rn);
        mln_rbtree_node_free(conf_set, rn);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

/* ---- conf_free ----------------------------------------------------------- */

static int mln_lang_tls_conf_free_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "conf_free", mln_lang_tls_conf_free_process, { "conf" }, 1,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_conf_free_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_conf = mln_string("conf");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    mln_lang_tls_conf_t tmp;
    mln_rbtree_t *conf_set;
    mln_rbtree_node_t *rn;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_conf, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'conf' must be an int returned by conf_new."); return NULL; }
    tmp.id   = sym->data.var->val->data.i;
    tmp.conf = NULL;
    conf_set = mln_lang_resource_fetch(ctx->lang, "tls_conf");
    if (conf_set != NULL) {
        rn = mln_rbtree_search(conf_set, &tmp);
        if (!mln_rbtree_null(rn, conf_set)) {
            mln_rbtree_delete(conf_set, rn);
            mln_rbtree_node_free(conf_set, rn);
        }
    }
    if ((ret_var = mln_lang_var_create_nil(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

/* ---- listen -------------------------------------------------------------- */

static int mln_lang_tls_listen_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "listen", mln_lang_tls_listen_process, { "host", "service" }, 2,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_listen_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_host = mln_string("host"), v_svc = mln_string("service");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    struct addrinfo hints, *res = NULL;
    char host[128] = {0}, service[64] = {0}, ip[128] = {0};
    int fd = -1, opt = 1;
    mln_u16_t port = 0;
    mln_lang_tls_t *lt;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_host, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'host' must be a string."); return NULL; }
    if (sym->data.var->val->data.s->len > sizeof(host) - 1) {
        mln_lang_errmsg(ctx, "host too long."); return NULL;
    }
    memcpy(host, sym->data.var->val->data.s->data, sym->data.var->val->data.s->len);

    if ((sym = mln_lang_symbol_node_search(ctx, &v_svc, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'service' must be a string."); return NULL; }
    if (sym->data.var->val->data.s->len > sizeof(service) - 1) {
        mln_lang_errmsg(ctx, "service too long."); return NULL;
    }
    memcpy(service, sym->data.var->val->data.s->data, sym->data.var->val->data.s->len);

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;
    if (getaddrinfo(host, service, &hints, &res) != 0 || res == NULL) goto fail_false;
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        goto fail_false;
    if (mln_lang_tls_get_addr(res->ai_addr, ip, &port) < 0) goto fail_false;
#if defined(WIN32)
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
        goto fail_false;
#else
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        goto fail_false;
#endif
#if !defined(WIN32) && !defined(MLN_C99)
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        goto fail_false;
#endif
    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) goto fail_false;
    if (listen(fd, 32767) < 0) goto fail_false;

    if ((lt = mln_lang_tls_new_listen(ctx->lang, fd, ip, port)) == NULL) {
        mln_socket_close(fd); fd = -1;
        freeaddrinfo(res); res = NULL;
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (mln_lang_tls_resource_add(ctx->lang, lt) < 0) {
        mln_lang_tls_free(lt);
        freeaddrinfo(res); res = NULL;
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    freeaddrinfo(res); res = NULL;

    if ((ret_var = mln_lang_var_create_int(ctx, fd, NULL)) == NULL) {
        mln_lang_tls_resource_remove(ctx->lang, fd);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;

fail_false:
    if (fd >= 0) mln_socket_close(fd);
    if (res != NULL) freeaddrinfo(res);
    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

/* ---- accept -------------------------------------------------------------- */

static int mln_lang_tls_accept_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "accept", mln_lang_tls_accept_process,
        { "fd", "conf", "timeout" }, 3,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_accept_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_fd = mln_string("fd"), v_conf = mln_string("conf"), v_to = mln_string("timeout");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    int fd, timeout;
    mln_s64_t conf_id;
    mln_lang_tls_t *lt;
    mln_lang_tls_conf_t *cnode;
    struct mln_lang_tls_accept_req *req;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_fd, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'fd' must be an int."); return NULL; }
    fd = (int)sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_conf, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'conf' must be an int."); return NULL; }
    conf_id = sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_to, 1)) == NULL) {
        mln_lang_errmsg(ctx, "Argument 'timeout' missing."); return NULL;
    } else {
        mln_u32_t t = mln_lang_var_val_type_get(sym->data.var);
        if (t == M_LANG_VAL_TYPE_NIL) timeout = M_EV_UNLIMITED;
        else if (t == M_LANG_VAL_TYPE_INT && sym->data.var->val->data.i >= 0)
            timeout = (int)sym->data.var->val->data.i;
        else {
            mln_lang_errmsg(ctx, "Argument 'timeout' must be a non-negative int or nil.");
            return NULL;
        }
    }

    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    if ((lt = mln_lang_tls_resource_fetch(ctx->lang, fd)) == NULL) return ret_var;
    if (!lt->is_listen) {
        mln_lang_errmsg(ctx, "fd is not a listen socket.");
        return ret_var;
    }
    if ((cnode = mln_lang_tls_conf_lookup(ctx->lang, conf_id)) == NULL) {
        mln_lang_errmsg(ctx, "Invalid 'conf' handle.");
        return ret_var;
    }
    (void)cnode;
    if (lt->recving) {
        mln_lang_errmsg(ctx, "Listen socket already accepting in another script task.");
        return ret_var;
    }

    req = (struct mln_lang_tls_accept_req_s *)malloc(sizeof(*req));
    if (req == NULL) {
        mln_lang_var_free(ret_var);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    req->lt      = lt;
    req->conf_id = conf_id;
    lt->timeout  = timeout;
    lt->accept_req = req;

    if (mln_event_fd_set(ctx->lang->ev, fd,
                         M_EV_RECV|M_EV_NONBLOCK|M_EV_ONESHOT,
                         timeout, req,
                         mln_lang_tls_accept_handler) < 0) {
        lt->accept_req = NULL;
        free(req);
        mln_lang_var_free(ret_var);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (timeout != M_EV_UNLIMITED) {
        mln_event_fd_timeout_handler_set(ctx->lang->ev, fd, req,
                                         mln_lang_tls_accept_timeout_handler);
    }
    lt->recving = 1;
    mln_lang_ctx_tls_resource_add(ctx, lt);
    mln_lang_ctx_suspend(ctx);
    return ret_var;
}

static void mln_lang_tls_accept_handler(mln_event_t *ev, int fd, void *data)
{
    struct mln_lang_tls_accept_req_s *req = (struct mln_lang_tls_accept_req_s *)data;
    mln_lang_tls_t *lt = req->lt;
    mln_lang_t *lang = lt->lang;
    int connfd;
    mln_u8_t addr[sizeof(struct sockaddr) >= sizeof(struct sockaddr_in6) ?
                  sizeof(struct sockaddr) : sizeof(struct sockaddr_in6)];
    char ip[128] = {0};
    mln_u16_t port = 0;
    socklen_t len = sizeof(addr);
    mln_lang_var_t *ret_var = NULL;
    mln_lang_tls_t *new_lt = NULL;
    mln_lang_tls_conf_t *cnode;

    memset(addr, 0, sizeof(addr));
    connfd = accept(fd, (struct sockaddr *)addr, &len);

    mln_lang_mutex_lock(lang);
    if (connfd < 0) {
        if (errno == EINTR || errno == EAGAIN
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
            || errno == EWOULDBLOCK
#endif
            ) {
            /* Spurious wake-up; re-arm. */
            mln_event_fd_set(ev, fd,
                             M_EV_RECV|M_EV_NONBLOCK|M_EV_ONESHOT,
                             lt->timeout, req,
                             mln_lang_tls_accept_handler);
            if (lt->timeout != M_EV_UNLIMITED) {
                mln_event_fd_timeout_handler_set(ev, fd, req,
                                                 mln_lang_tls_accept_timeout_handler);
            }
            mln_lang_mutex_unlock(lang);
            return;
        }
        goto resume_with_pre_var;
    }

    /* Resolve conf after re-acquiring the lock so a concurrent
     * conf_free cannot release the SSL_CTX between request creation
     * and use. */
    if ((cnode = mln_lang_tls_conf_lookup(lang, req->conf_id)) == NULL) {
        mln_socket_close(connfd);
        goto resume_with_pre_var;
    }
    if (mln_lang_tls_get_addr(addr, ip, &port) < 0) {
        mln_socket_close(connfd);
        goto resume_with_pre_var;
    }
    if ((new_lt = mln_lang_tls_new_conn(lang, connfd, cnode->conf, ip, port)) == NULL) {
        mln_socket_close(connfd);
        goto resume_with_pre_var;
    }
    if (mln_lang_tls_resource_add(lang, new_lt) < 0) {
        mln_lang_tls_free(new_lt);
        goto resume_with_pre_var;
    }
    if ((ret_var = mln_lang_var_create_int(lt->ctx, connfd, NULL)) == NULL) {
        mln_lang_tls_resource_remove(lang, connfd);
        goto resume_with_pre_var;
    }
    mln_lang_ctx_set_ret_var(lt->ctx, ret_var);

resume_with_pre_var:
    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    lt->recving = 0;
    /* Clear the back-pointer before freeing the bag: this prevents a
     * concurrent tls_free from following a dangling pointer. */
    lt->accept_req = NULL;
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    free(req);
    mln_lang_mutex_unlock(lang);
}

static void mln_lang_tls_accept_timeout_handler(mln_event_t *ev, int fd, void *data)
{
    struct mln_lang_tls_accept_req_s *req = (struct mln_lang_tls_accept_req_s *)data;
    mln_lang_tls_t *lt = req->lt;
    mln_lang_t *lang = lt->lang;
    mln_lang_var_t *ret_var;
    mln_lang_mutex_lock(lang);
    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    lt->recving = 0;
    lt->accept_req = NULL;
    if ((ret_var = mln_lang_var_create_nil(lt->ctx, NULL)) != NULL)
        mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    free(req);
    mln_lang_mutex_unlock(lang);
}

/* ---- connect ------------------------------------------------------------- */

static int mln_lang_tls_connect_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "connect", mln_lang_tls_connect_process,
        { "host", "service", "conf", "timeout" }, 4,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_connect_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_host = mln_string("host"), v_svc = mln_string("service");
    mln_string_t v_conf = mln_string("conf"), v_to  = mln_string("timeout");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    struct addrinfo hints, *res = NULL;
    char host[128] = {0}, service[64] = {0}, ip[128] = {0};
    int fd = -1, timeout;
    mln_u16_t port = 0;
    mln_s64_t conf_id;
    mln_lang_tls_t *lt = NULL;
    mln_lang_tls_conf_t *cnode;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_host, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'host' must be a string."); return NULL; }
    if (sym->data.var->val->data.s->len > sizeof(host) - 1) {
        mln_lang_errmsg(ctx, "host too long."); return NULL;
    }
    memcpy(host, sym->data.var->val->data.s->data, sym->data.var->val->data.s->len);

    if ((sym = mln_lang_symbol_node_search(ctx, &v_svc, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'service' must be a string."); return NULL; }
    if (sym->data.var->val->data.s->len > sizeof(service) - 1) {
        mln_lang_errmsg(ctx, "service too long."); return NULL;
    }
    memcpy(service, sym->data.var->val->data.s->data, sym->data.var->val->data.s->len);

    if ((sym = mln_lang_symbol_node_search(ctx, &v_conf, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'conf' must be an int."); return NULL; }
    conf_id = sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_to, 1)) == NULL) {
        mln_lang_errmsg(ctx, "Argument 'timeout' missing."); return NULL;
    } else {
        mln_u32_t t = mln_lang_var_val_type_get(sym->data.var);
        if (t == M_LANG_VAL_TYPE_NIL) timeout = M_EV_UNLIMITED;
        else if (t == M_LANG_VAL_TYPE_INT && sym->data.var->val->data.i >= 0)
            timeout = (int)sym->data.var->val->data.i;
        else {
            mln_lang_errmsg(ctx, "Argument 'timeout' must be a non-negative int or nil.");
            return NULL;
        }
    }

    if ((cnode = mln_lang_tls_conf_lookup(ctx->lang, conf_id)) == NULL) {
        mln_lang_errmsg(ctx, "Invalid 'conf' handle.");
        return NULL;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;
    if (getaddrinfo(host, service, &hints, &res) != 0 || res == NULL) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        goto fail_false;
    if (mln_lang_tls_get_addr(res->ai_addr, ip, &port) < 0) goto fail_false;

    if ((lt = mln_lang_tls_new_conn(ctx->lang, fd, cnode->conf, ip, port)) == NULL) {
        mln_socket_close(fd); fd = -1;
        freeaddrinfo(res); res = NULL;
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    lt->connect_timeout = timeout;
    if (mln_event_fd_set(ctx->lang->ev, fd,
                         M_EV_SEND|M_EV_ERROR|M_EV_NONBLOCK|M_EV_ONESHOT,
                         timeout, lt,
                         mln_lang_tls_connect_handler) < 0) {
        mln_lang_tls_free(lt);
        freeaddrinfo(res); res = NULL;
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
#if defined(WIN32)
    if (connect(fd, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR
        && WSAGetLastError() != WSAEWOULDBLOCK) {
#else
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0
        && errno != EINPROGRESS) {
#endif
        mln_event_fd_set(ctx->lang->ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
        mln_lang_tls_free(lt);
        freeaddrinfo(res); res = NULL;
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    freeaddrinfo(res); res = NULL;

    if (mln_lang_tls_resource_add(ctx->lang, lt) < 0) {
        mln_event_fd_set(ctx->lang->ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
        mln_lang_tls_free(lt);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_event_fd_set(ctx->lang->ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
        mln_lang_tls_resource_remove(ctx->lang, fd);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (timeout != M_EV_UNLIMITED) {
        mln_event_fd_timeout_handler_set(ctx->lang->ev, fd, lt,
                                         mln_lang_tls_connect_timeout_handler);
    }
    lt->sending = 1;
    mln_lang_ctx_tls_resource_add(ctx, lt);
    mln_lang_ctx_suspend(ctx);
    return ret_var;

fail_false:
    if (fd >= 0) mln_socket_close(fd);
    if (res != NULL) freeaddrinfo(res);
    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static void mln_lang_tls_connect_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)data;
    mln_lang_t *lang = lt->lang;
    int err = 0;
    socklen_t errlen = sizeof(err);
    mln_lang_var_t *ret_var;

    mln_lang_mutex_lock(lang);
#if defined(WIN32)
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&err, &errlen) < 0) {
        err = WSAGetLastError() ? WSAGetLastError() : EIO;
    }
#else
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0) {
        err = errno ? errno : EIO;
    }
#endif
    if (err == 0) {
        if ((ret_var = mln_lang_var_create_int(lt->ctx, fd, NULL)) != NULL) {
            mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
        }
        mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
        lt->sending = 0;
        mln_lang_ctx_continue(lt->ctx);
        mln_lang_ctx_tls_resource_remove(lt);
        mln_lang_mutex_unlock(lang);
        return;
    }
    if (err == EINPROGRESS && (++lt->retry <= MLN_LANG_TLS_CONNECT_RETRY)) {
        mln_event_fd_set(ev, fd,
                         M_EV_SEND|M_EV_ERROR|M_EV_NONBLOCK|M_EV_ONESHOT,
                         lt->connect_timeout, lt,
                         mln_lang_tls_connect_handler);
        if (lt->connect_timeout != M_EV_UNLIMITED) {
            mln_event_fd_timeout_handler_set(ev, fd, lt,
                                             mln_lang_tls_connect_timeout_handler);
        }
        mln_lang_mutex_unlock(lang);
        return;
    }
    /* Fatal connect failure: tear the resource down so the fd is closed
     * exactly once.  Doing this here also frees the half-built lt; any
     * subsequent script-side send/recv on its fd will simply observe
     * "fd not found" and return false. */
    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    lt->sending = 0;
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    mln_lang_tls_resource_remove(lang, fd);
    mln_lang_mutex_unlock(lang);
}

static void mln_lang_tls_connect_timeout_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)data;
    mln_lang_t *lang = lt->lang;
    mln_lang_var_t *ret_var;

    mln_lang_mutex_lock(lang);
    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    lt->sending = 0;
    if ((ret_var = mln_lang_var_create_nil(lt->ctx, NULL)) != NULL) {
        mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
    }
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    mln_lang_tls_resource_remove(lang, fd);
    mln_lang_mutex_unlock(lang);
}

/* ---- set_sni / set_verify_host ------------------------------------------ */

static int mln_lang_tls_set_sni_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "set_sni", mln_lang_tls_set_sni_process, { "fd", "hostname" }, 2,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static int mln_lang_tls_set_verify_host_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "set_verify_host", mln_lang_tls_set_verify_host_process,
        { "fd", "hostname" }, 2,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_set_sni_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_fd = mln_string("fd"), v_h = mln_string("hostname");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    mln_lang_tls_t *lt;
    int fd;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_fd, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'fd' must be an int."); return NULL; }
    fd = (int)sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_h, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'hostname' must be a string."); return NULL; }

    if ((lt = mln_lang_tls_resource_fetch(ctx->lang, fd)) == NULL || lt->is_listen) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    if (mln_tcp_conn_tls_set_sni(&lt->conn, sym->data.var->val->data.s) < 0) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    if ((ret_var = mln_lang_var_create_true(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static mln_lang_var_t *mln_lang_tls_set_verify_host_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_fd = mln_string("fd"), v_h = mln_string("hostname");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    mln_lang_tls_t *lt;
    int fd;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_fd, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'fd' must be an int."); return NULL; }
    fd = (int)sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_h, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'hostname' must be a string."); return NULL; }

    if ((lt = mln_lang_tls_resource_fetch(ctx->lang, fd)) == NULL || lt->is_listen) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    if (mln_tcp_conn_tls_set_verify_host(&lt->conn, sym->data.var->val->data.s) < 0) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    if ((ret_var = mln_lang_var_create_true(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

/* ---- handshake / send / recv (the shared TLS state machine) ------------- */

static int mln_lang_tls_rearm(mln_lang_tls_t *lt, mln_event_t *ev,
                              mln_u32_t fallback_flag,
                              int timeout,
                              ev_fd_handler handler)
{
    int fd = mln_tcp_conn_fd_get(&lt->conn);
    mln_u32_t flag = 0;
    if (mln_tcp_conn_tls_want_read(&lt->conn))  flag |= M_EV_RECV;
    if (mln_tcp_conn_tls_want_write(&lt->conn)) flag |= M_EV_SEND;
    if (flag == 0) flag = fallback_flag;
    if (mln_event_fd_set(ev, fd,
                         flag|M_EV_NONBLOCK|M_EV_ONESHOT,
                         timeout, lt, handler) < 0)
        return -1;
    if (timeout != M_EV_UNLIMITED) {
        mln_event_fd_timeout_handler_set(ev, fd, lt, mln_lang_tls_timeout_handler);
    }
    return 0;
}

/* handshake */

static int mln_lang_tls_handshake_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "handshake", mln_lang_tls_handshake_process, { "fd", "timeout" }, 2,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_handshake_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_fd = mln_string("fd"), v_to = mln_string("timeout");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    int fd, timeout, r;
    mln_lang_tls_t *lt;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_fd, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'fd' must be an int."); return NULL; }
    fd = (int)sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_to, 1)) == NULL) {
        mln_lang_errmsg(ctx, "Argument 'timeout' missing."); return NULL;
    } else {
        mln_u32_t t = mln_lang_var_val_type_get(sym->data.var);
        if (t == M_LANG_VAL_TYPE_NIL) timeout = M_EV_UNLIMITED;
        else if (t == M_LANG_VAL_TYPE_INT && sym->data.var->val->data.i >= 0)
            timeout = (int)sym->data.var->val->data.i;
        else {
            mln_lang_errmsg(ctx, "Argument 'timeout' must be a non-negative int or nil.");
            return NULL;
        }
    }

    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    if ((lt = mln_lang_tls_resource_fetch(ctx->lang, fd)) == NULL) return ret_var;
    if (lt->is_listen) {
        mln_lang_errmsg(ctx, "fd is a listen socket; cannot handshake.");
        return ret_var;
    }
    if (lt->sending || lt->recving || lt->shutting) {
        mln_lang_errmsg(ctx, "Socket busy in another script task.");
        return ret_var;
    }

    if (mln_tcp_conn_tls_done(&lt->conn)) {
        mln_lang_var_free(ret_var);
        if ((ret_var = mln_lang_var_create_true(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }

    /* First synchronous attempt -- if it completes inline, never touch
     * the event loop at all and the script keeps running. */
    r = mln_tcp_conn_tls_handshake(&lt->conn);
    if (r == M_C_FINISH) {
        mln_lang_var_free(ret_var);
        if ((ret_var = mln_lang_var_create_true(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    if (r == M_C_ERROR || r == M_C_CLOSED) return ret_var;

    /* M_C_NOTYET: suspend. */
    lt->timeout = timeout;
    if (mln_lang_tls_rearm(lt, ctx->lang->ev,
                           M_EV_RECV, timeout,
                           mln_lang_tls_handshake_handler) < 0) {
        mln_lang_var_free(ret_var);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    lt->recving = 1;
    mln_lang_ctx_tls_resource_add(ctx, lt);
    mln_lang_ctx_suspend(ctx);
    return ret_var;
}

static void mln_lang_tls_handshake_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)data;
    mln_lang_t *lang = lt->lang;
    int r;
    mln_lang_var_t *ret_var;

    r = mln_tcp_conn_tls_handshake(&lt->conn);

    mln_lang_mutex_lock(lang);
    if (r == M_C_NOTYET) {
        if (mln_lang_tls_rearm(lt, ev, M_EV_RECV, lt->timeout,
                               mln_lang_tls_handshake_handler) == 0) {
            mln_lang_mutex_unlock(lang);
            return;
        }
        /* rearm failed -- fall through to terminal false. */
    }
    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    if (r == M_C_FINISH) {
        if ((ret_var = mln_lang_var_create_true(lt->ctx, NULL)) != NULL)
            mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
    }
    /* On error/closed, the pre-suspend `false` stays. */
    lt->recving = 0;
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    mln_lang_mutex_unlock(lang);
}

/* send */

static int mln_lang_tls_send_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "send", mln_lang_tls_send_process, { "fd", "data", "timeout" }, 3,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_send_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_fd = mln_string("fd"), v_data = mln_string("data"), v_to = mln_string("timeout");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    mln_lang_tls_t *lt;
    int fd, timeout;
    mln_string_t *data;
    mln_chain_t *c;
    mln_buf_t *b;
    mln_u8ptr_t buf;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_fd, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'fd' must be an int."); return NULL; }
    fd = (int)sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_data, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    { mln_lang_errmsg(ctx, "Argument 'data' must be a string."); return NULL; }
    data = sym->data.var->val->data.s;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_to, 1)) == NULL) {
        mln_lang_errmsg(ctx, "Argument 'timeout' missing."); return NULL;
    } else {
        mln_u32_t t = mln_lang_var_val_type_get(sym->data.var);
        if (t == M_LANG_VAL_TYPE_NIL) timeout = M_EV_UNLIMITED;
        else if (t == M_LANG_VAL_TYPE_INT && sym->data.var->val->data.i >= 0)
            timeout = (int)sym->data.var->val->data.i;
        else {
            mln_lang_errmsg(ctx, "Argument 'timeout' must be a non-negative int or nil.");
            return NULL;
        }
    }

    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    if ((lt = mln_lang_tls_resource_fetch(ctx->lang, fd)) == NULL) return ret_var;
    if (lt->is_listen) {
        mln_lang_errmsg(ctx, "Cannot send on a listen socket.");
        return ret_var;
    }
    if (lt->sending || lt->recving || lt->shutting) {
        mln_lang_errmsg(ctx, "Socket busy in another script task.");
        return ret_var;
    }
    if (lt->send_closed) {
        mln_lang_errmsg(ctx, "Socket send already shut down.");
        return ret_var;
    }
    if (data->len == 0) {
        mln_lang_var_free(ret_var);
        if ((ret_var = mln_lang_var_create_true(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }

    /* Own a copy of the caller's bytes: the caller's mln_string_t may
     * be reclaimed before the event loop drains the send queue. */
    mln_alloc_t *pool = mln_tcp_conn_pool_get(&lt->conn);
    if ((c = mln_chain_new(pool)) == NULL) goto nomem;
    if ((b = mln_buf_new(pool)) == NULL) {
        mln_chain_pool_release(c);
        goto nomem;
    }
    c->buf = b;
    if ((buf = (mln_u8ptr_t)mln_alloc_m(pool, data->len)) == NULL) {
        mln_chain_pool_release(c);
        goto nomem;
    }
    memcpy(buf, data->data, data->len);
    b->left_pos = b->pos = b->start = buf;
    b->last     = b->end             = buf + data->len;
    b->in_memory     = 1;
    b->last_buf      = 1;
    b->last_in_chain = 1;
    mln_tcp_conn_append(&lt->conn, c, M_C_SEND);

    /* Fast path: attempt the send synchronously.  For small payloads
     * that fit in the socket buffer this completes inline without ever
     * touching the event loop, which is by far the common case for
     * request/response style traffic.  It is also the only correct
     * place to evaluate the TLS want flags: those flags can carry
     * stale state from a prior recv (mln_tcp_conn_recv_tls returns
     * NOTYET with tls_want_r=1 even after a successful SSL_read), and
     * mln_tcp_conn_send_tls clears them on entry. */
    {
        int r = mln_tcp_conn_send(&lt->conn);
        mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_SENT));
        if (r == M_C_ERROR || r == M_C_CLOSED) {
            mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_SEND));
            return ret_var; /* false */
        }
        if (r == M_C_FINISH
            || (mln_tcp_conn_head(&lt->conn, M_C_SEND) == NULL
                && !mln_tcp_conn_tls_want_read(&lt->conn)
                && !mln_tcp_conn_tls_want_write(&lt->conn)))
        {
            mln_lang_var_free(ret_var);
            if ((ret_var = mln_lang_var_create_true(ctx, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        }
    }

    lt->timeout = timeout;
    if (mln_lang_tls_rearm(lt, ctx->lang->ev,
                           M_EV_SEND, timeout,
                           mln_lang_tls_send_handler) < 0) {
        mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_SEND));
        goto nomem;
    }
    lt->sending = 1;
    mln_lang_ctx_tls_resource_add(ctx, lt);
    mln_lang_ctx_suspend(ctx);
    return ret_var;

nomem:
    mln_lang_var_free(ret_var);
    mln_lang_errmsg(ctx, "No memory.");
    return NULL;
}

static void mln_lang_tls_send_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)data;
    mln_lang_t *lang = lt->lang;
    int r;
    int rearm_failed = 0;
    mln_lang_var_t *ret_var;

    r = mln_tcp_conn_send(&lt->conn);

    mln_lang_mutex_lock(lang);
    /* Release ciphertext+plaintext that has been fully drained. */
    mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_SENT));

    if (r == M_C_FINISH) {
        if ((ret_var = mln_lang_var_create_true(lt->ctx, NULL)) != NULL)
            mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
    } else if (r == M_C_NOTYET) {
        /* If the send queue is empty and TLS doesn't want any I/O, treat
         * as completion (mln_tcp_conn_send_tls reports NOTYET after the
         * final write because is_done isn't set by the caller). */
        if (mln_tcp_conn_head(&lt->conn, M_C_SEND) == NULL
            && !mln_tcp_conn_tls_want_read(&lt->conn)
            && !mln_tcp_conn_tls_want_write(&lt->conn))
        {
            if ((ret_var = mln_lang_var_create_true(lt->ctx, NULL)) != NULL)
                mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
        } else {
            if (mln_lang_tls_rearm(lt, ev, M_EV_SEND, lt->timeout,
                                   mln_lang_tls_send_handler) == 0) {
                mln_lang_mutex_unlock(lang);
                return;
            }
            rearm_failed = 1;
        }
    }
    /* M_C_ERROR/M_C_CLOSED: keep the pre-suspend false. */

    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    /* Drop any partially-sent SEND queue so the next operation does not
     * inherit half-encrypted plaintext: this matters not only on
     * ERROR/CLOSED but also on a rearm failure mid-NOTYET, which is
     * functionally a hard failure too. */
    if (r == M_C_ERROR || r == M_C_CLOSED || rearm_failed) {
        mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_SEND));
    }
    lt->sending = 0;
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    mln_lang_mutex_unlock(lang);
}

/* recv */

static int mln_lang_tls_recv_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "recv", mln_lang_tls_recv_process, { "fd", "timeout" }, 2,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_recv_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_fd = mln_string("fd"), v_to = mln_string("timeout");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    mln_lang_tls_t *lt;
    int fd, timeout;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_fd, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'fd' must be an int."); return NULL; }
    fd = (int)sym->data.var->val->data.i;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_to, 1)) == NULL) {
        mln_lang_errmsg(ctx, "Argument 'timeout' missing."); return NULL;
    } else {
        mln_u32_t t = mln_lang_var_val_type_get(sym->data.var);
        if (t == M_LANG_VAL_TYPE_NIL) timeout = M_EV_UNLIMITED;
        else if (t == M_LANG_VAL_TYPE_INT && sym->data.var->val->data.i >= 0)
            timeout = (int)sym->data.var->val->data.i;
        else {
            mln_lang_errmsg(ctx, "Argument 'timeout' must be a non-negative int or nil.");
            return NULL;
        }
    }

    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    if ((lt = mln_lang_tls_resource_fetch(ctx->lang, fd)) == NULL) return ret_var;
    if (lt->is_listen) {
        mln_lang_errmsg(ctx, "Cannot recv on a listen socket; use accept.");
        return ret_var;
    }
    if (lt->sending || lt->recving || lt->shutting) {
        mln_lang_errmsg(ctx, "Socket busy in another script task.");
        return ret_var;
    }
    if (lt->recv_closed) {
        mln_lang_errmsg(ctx, "Socket recv already shut down.");
        return ret_var;
    }

    /* Fast path: try a non-blocking recv inline.  If decrypted bytes
     * are already pending in rbio or the socket has bytes ready, we
     * return them without paying for an event-loop trip.  Calling
     * mln_tcp_conn_recv_tls also resets the TLS want flags on entry,
     * which is necessary because a previous send may have left
     * want_w=1 set (e.g. mid-renegotiation). */
    {
        int r = mln_tcp_conn_recv(&lt->conn, M_C_TYPE_MEMORY);
        if (r == M_C_ERROR) {
            return ret_var; /* false */
        }
        mln_chain_t *c = mln_tcp_conn_head(&lt->conn, M_C_RECV);
        if (c != NULL) {
            mln_s64_t size = 0;
            for (; c != NULL; c = c->next) {
                if (c->buf == NULL) continue;
                size += (mln_s64_t)mln_buf_left_size(c->buf);
            }
            if (size > 0) {
                mln_u8ptr_t fbuf = (mln_u8ptr_t)malloc((size_t)size);
                if (fbuf == NULL) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                mln_u8ptr_t p = fbuf;
                for (c = mln_tcp_conn_head(&lt->conn, M_C_RECV); c != NULL; c = c->next) {
                    if (c->buf == NULL) continue;
                    size_t n = mln_buf_left_size(c->buf);
                    memcpy(p, c->buf->left_pos, n);
                    p += n;
                }
                mln_string_t tmp;
                mln_string_nset(&tmp, fbuf, (mln_size_t)size);
                mln_lang_var_t *new_ret = mln_lang_var_create_string(ctx, &tmp, NULL);
                free(fbuf);
                mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_RECV));
                if (new_ret == NULL) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                mln_lang_var_free(ret_var);
                return new_ret;
            }
        }
        if (r == M_C_CLOSED) {
            mln_lang_var_free(ret_var);
            if ((ret_var = mln_lang_var_create_true(ctx, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        }
        /* M_C_NOTYET and no data: fall through to suspend. */
    }

    lt->timeout = timeout;
    if (mln_lang_tls_rearm(lt, ctx->lang->ev,
                           M_EV_RECV, timeout,
                           mln_lang_tls_recv_handler) < 0) {
        mln_lang_var_free(ret_var);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    lt->recving = 1;
    mln_lang_ctx_tls_resource_add(ctx, lt);
    mln_lang_ctx_suspend(ctx);
    return ret_var;
}

static void mln_lang_tls_recv_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)data;
    mln_lang_t *lang = lt->lang;
    int r;
    mln_lang_var_t *ret_var;

    r = mln_tcp_conn_recv(&lt->conn, M_C_TYPE_MEMORY);

    mln_lang_mutex_lock(lang);
    if (r == M_C_NOTYET && mln_tcp_conn_head(&lt->conn, M_C_RECV) == NULL) {
        if (mln_lang_tls_rearm(lt, ev, M_EV_RECV, lt->timeout,
                               mln_lang_tls_recv_handler) == 0) {
            mln_lang_mutex_unlock(lang);
            return;
        }
        r = M_C_ERROR;
    }

    /* Coalesce the recv queue into a single Melang string. */
    mln_chain_t *c = mln_tcp_conn_head(&lt->conn, M_C_RECV);
    mln_s64_t size = 0;
    for (; c != NULL; c = c->next) {
        if (c->buf == NULL) continue;
        size += (mln_s64_t)mln_buf_left_size(c->buf);
    }

    if (size > 0) {
        mln_u8ptr_t buf = (mln_u8ptr_t)malloc((size_t)size);
        if (buf == NULL) {
            mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
            lt->recving = 0;
            mln_lang_ctx_continue(lt->ctx);
            mln_lang_ctx_tls_resource_remove(lt);
            mln_lang_mutex_unlock(lang);
            return;
        }
        mln_u8ptr_t p = buf;
        for (c = mln_tcp_conn_head(&lt->conn, M_C_RECV); c != NULL; c = c->next) {
            if (c->buf == NULL) continue;
            size_t n = mln_buf_left_size(c->buf);
            memcpy(p, c->buf->left_pos, n);
            p += n;
        }
        mln_string_t tmp;
        mln_string_nset(&tmp, buf, (mln_size_t)size);
        ret_var = mln_lang_var_create_string(lt->ctx, &tmp, NULL);
        free(buf);
        if (ret_var != NULL) {
            mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
        }
        mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_RECV));
    } else if (r == M_C_CLOSED || r == M_C_FINISH) {
        if ((ret_var = mln_lang_var_create_true(lt->ctx, NULL)) != NULL)
            mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
    }
    /* M_C_ERROR with empty queue: keep the pre-suspend false. */

    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    lt->recving = 0;
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    mln_lang_mutex_unlock(lang);
}

/* Generic per-fd timeout used by handshake/send/recv (not by accept,
 * which carries an extra request bag).  On timeout we drop any
 * partially-queued send buffers so that the next send call starts
 * with a clean queue -- leaving a half-sent record there would
 * corrupt the wire stream for the next operation. */
static void mln_lang_tls_timeout_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_tls_t *lt = (mln_lang_tls_t *)data;
    mln_lang_t *lang = lt->lang;
    mln_lang_var_t *ret_var;
    mln_lang_mutex_lock(lang);
    mln_event_fd_set(ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    /* Drop any half-sent queues so a subsequent send/recv doesn't
     * inherit stale plaintext or sent-but-unreleased chains. */
    mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_SEND));
    mln_chain_pool_release_all(mln_tcp_conn_remove(&lt->conn, M_C_SENT));
    lt->sending = lt->recving = lt->shutting = 0;
    if ((ret_var = mln_lang_var_create_nil(lt->ctx, NULL)) != NULL)
        mln_lang_ctx_set_ret_var(lt->ctx, ret_var);
    mln_lang_ctx_continue(lt->ctx);
    mln_lang_ctx_tls_resource_remove(lt);
    mln_lang_mutex_unlock(lang);
}

/* ---- close --------------------------------------------------------------- */

static int mln_lang_tls_close_reg(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    static const mln_lang_tls_funcdef_t def = {
        "close", mln_lang_tls_close_process, { "fd" }, 1,
    };
    return mln_lang_tls_register_func(ctx, obj, &def);
}

static mln_lang_var_t *mln_lang_tls_close_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_fd = mln_string("fd");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var;
    mln_lang_tls_t *lt;
    int fd;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_fd, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    { mln_lang_errmsg(ctx, "Argument 'fd' must be an int."); return NULL; }
    fd = (int)sym->data.var->val->data.i;

    if ((ret_var = mln_lang_var_create_nil(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    /* Best-effort graceful shutdown: one non-blocking SSL_shutdown
     * round.  Returning NOTYET is fine -- we still drop the connection
     * unconditionally below.  This keeps close() synchronous; scripts
     * that need a strict bidirectional close_notify can call recv()
     * until it returns true before closing. */
    if ((lt = mln_lang_tls_resource_fetch(ctx->lang, fd)) != NULL
        && !lt->is_listen && mln_tcp_conn_tls_enabled(&lt->conn))
    {
        if (!lt->sending && !lt->recving && !lt->shutting) {
            (void)mln_tcp_conn_tls_shutdown(&lt->conn);
        }
    }
    mln_lang_tls_resource_remove(ctx->lang, fd);
    return ret_var;
}

#endif /* MLN_TLS */
