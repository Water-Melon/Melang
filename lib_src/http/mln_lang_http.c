
/*
 * Copyright (C) Niklaus F.Schen.
 *
 * Note: not support chunk mode.
 */
#include "mln_lang_http.h"
#include "mln_http.h"
#include "mln_chain.h"
#include "mln_conf.h"
#include "mln_log.h"
#if defined(MLN_TLS)
#if defined(WIN32)
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "tls/mln_lang_tls.h"
#endif

static int mln_lang_http(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_http_parse_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static mln_lang_var_t *mln_lang_http_parse_process(mln_lang_ctx_t *ctx);
static int mln_lang_http_parse_body_handler(mln_http_t *http, mln_chain_t **in, mln_chain_t **nil);
static inline mln_lang_var_t *mln_lang_http_parse_result_build(mln_lang_ctx_t *ctx, mln_http_t *http);
static inline int mln_lang_http_parse_request_result_build(mln_lang_ctx_t *ctx, mln_http_t *http, mln_lang_array_t *arr);
static inline int mln_lang_http_parse_result_build_type(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_string_t *type);
static inline int mln_lang_http_parse_request_result_build_method(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_parse_request_result_build_version(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_parse_request_result_build_uri(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_parse_request_result_build_args(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_parse_result_build_headers(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static int mln_lang_http_parse_result_build_headers_iterate(mln_hash_t *h, void *k, void *v, void *udata);
static inline int mln_lang_http_parse_result_build_body(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_parse_response_result_build(mln_lang_ctx_t *ctx, mln_http_t *http, mln_lang_array_t *arr);
static inline int mln_lang_http_parse_response_result_build_version(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_parse_response_result_build_code(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);

static int mln_lang_http_create_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static mln_lang_var_t *mln_lang_http_create_process(mln_lang_ctx_t *ctx);
static int mln_lang_http_create_process_body_handler(mln_http_t *http, mln_chain_t **head, mln_chain_t **tail);
static inline mln_string_t *mln_lang_http_create_process_generate(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_create_process_generate_request_method(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_create_process_generate_version(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_create_process_generate_request_uri(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_create_process_generate_request_args(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline int mln_lang_http_create_process_generate_headers(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static int mln_lang_http_create_process_generate_headers_iterate(mln_rbtree_node_t *node, void *udata);
static inline int mln_lang_http_create_process_generate_response_code(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http);
static inline mln_string_t *mln_lang_http_create_process_generate_string(mln_lang_ctx_t *ctx, mln_chain_t *in);

mln_lang_var_t *init(mln_lang_ctx_t *ctx, mln_conf_t *cf)
{
    mln_log_init(cf);
    mln_lang_var_t *obj = mln_lang_var_create_obj(ctx, NULL, NULL);
    if (obj == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (mln_lang_http(ctx, mln_lang_var_val_get(obj)->data.obj) < 0) {
        mln_lang_var_free(obj);
        return NULL;
    }
    return obj;
}

#if defined(MLN_TLS)
static int mln_lang_http_https_request_register(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
#endif

static int mln_lang_http(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    if (mln_lang_http_parse_handler(ctx, obj) < 0) return -1;
    if (mln_lang_http_create_handler(ctx, obj) < 0) return -1;
#if defined(MLN_TLS)
    if (mln_lang_http_https_request_register(ctx, obj) < 0) return -1;
#endif
    return 0;
}

static int mln_lang_http_parse_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("parse");
    mln_string_t v1 = mln_string("data");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_http_parse_process, NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v1, M_LANG_VAR_REFER, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        mln_lang_func_detail_free(func);
        return -1;
    }
    mln_lang_func_detail_arg_append(func, var);
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_FUNC, func)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &funcname, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
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

static mln_lang_var_t *mln_lang_http_parse_process(mln_lang_ctx_t *ctx)
{
    int rc;
    mln_buf_t *b;
    mln_chain_t *c;
    mln_string_t *s;
    mln_http_t *http;
    mln_alloc_t *pool;
    mln_lang_var_t *var;
    mln_tcp_conn_t conn;
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("data");

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    var = sym->data.var;
    if (mln_lang_var_val_type_get(var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid argument 1.");
        return NULL;
    }
    if ((s = mln_lang_var_val_get(var)->data.s) == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument 1.");
        return NULL;
    }
    if (!s->len) {
        if ((ret_var = mln_lang_var_create_nil(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }

    if (mln_tcp_conn_init(&conn, -1) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    pool = mln_tcp_conn_pool_get(&conn);

    if ((c = mln_chain_new(pool)) == NULL) {
        mln_tcp_conn_destroy(&conn);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if ((b = mln_buf_new(pool)) == NULL) {
        mln_chain_pool_release(c);
        mln_tcp_conn_destroy(&conn);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    c->buf = b;
    b->left_pos = b->pos = b->start = s->data;
    b->last = b->end = s->data + s->len;
    b->temporary = 1;

    if ((http = mln_http_init(&conn, b, mln_lang_http_parse_body_handler)) == NULL) {
        mln_chain_pool_release(c);
        mln_tcp_conn_destroy(&conn);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if ((rc = mln_http_parse(http, &c)) == M_HTTP_RET_DONE) {
        if ((ret_var = mln_lang_http_parse_result_build(ctx, http)) == NULL) {
            mln_http_destroy(http);
            mln_chain_pool_release(c);
            mln_tcp_conn_destroy(&conn);
            return NULL;
        }
    } else if (rc == M_HTTP_RET_ERROR) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_http_destroy(http);
            mln_chain_pool_release(c);
            mln_tcp_conn_destroy(&conn);
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
    } else {/* rc == M_HTTP_RET_OK */
        if ((ret_var = mln_lang_var_create_nil(ctx, NULL)) == NULL) {
            mln_http_destroy(http);
            mln_chain_pool_release(c);
            mln_tcp_conn_destroy(&conn);
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
    }

    mln_http_destroy(http);
    if (rc == M_HTTP_RET_DONE && mln_buf_left_size(b) > 0) {
        mln_string_t tmp;
        mln_lang_var_t *v;

        tmp.data = b->left_pos;
        tmp.len = mln_buf_left_size(b);
        if ((v = mln_lang_var_create_string(ctx, &tmp, NULL)) == NULL) {
            mln_chain_pool_release(c);
            mln_tcp_conn_destroy(&conn);
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        mln_lang_var_value_set_string_ref(ctx, var, v);
        mln_lang_var_free(v);
    }
    mln_chain_pool_release(c);
    mln_tcp_conn_destroy(&conn);

    return ret_var;
}

static int mln_lang_http_parse_body_handler(mln_http_t *http, mln_chain_t **in, mln_chain_t **nil)
{
    mln_string_t cl_key = mln_string("Content-Length");
    mln_string_t *cl_val;
    mln_chain_t *c = *in;
    mln_sauto_t len, size = 0;

    cl_val = mln_http_field_get(http, &cl_key);
    if (cl_val == NULL) {
        return M_HTTP_RET_DONE;
    }

    len = (mln_sauto_t)atol((char *)(cl_val->data));
    if (!len) {
        return M_HTTP_RET_DONE;
    } else if (len < 0) {
        return M_HTTP_RET_ERROR;
    }

    for (; c != NULL; c = c->next) {
        size += mln_buf_left_size(c->buf);
        if (size >= len) break;
    }
    if (c == NULL)
        return M_HTTP_RET_OK;

    return M_HTTP_RET_DONE;
}

static inline mln_lang_var_t *mln_lang_http_parse_result_build(mln_lang_ctx_t *ctx, mln_http_t *http)
{
    mln_lang_var_t *ret_var;
    mln_lang_array_t *arr;

    if (mln_http_type_get(http) == M_HTTP_UNKNOWN) {
        mln_lang_errmsg(ctx, "Invalid HTTP data.");
        return NULL;
    }

    if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    arr = mln_lang_var_val_get(ret_var)->data.array;

    if (mln_http_type_get(http) == M_HTTP_REQUEST) {
        if (mln_lang_http_parse_request_result_build(ctx, http, arr) < 0) {
            mln_lang_var_free(ret_var);
            return NULL;
        }
    } else {
        if (mln_lang_http_parse_response_result_build(ctx, http, arr) < 0) {
            mln_lang_var_free(ret_var);
            return NULL;
        }
    }

    return ret_var;
}

static inline int mln_lang_http_parse_request_result_build(mln_lang_ctx_t *ctx, mln_http_t *http, mln_lang_array_t *arr)
{
    mln_string_t type = mln_string("request");

    if (mln_lang_http_parse_result_build_type(ctx, arr, &type) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_request_result_build_method(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_request_result_build_version(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_request_result_build_uri(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_request_result_build_args(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_result_build_headers(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_result_build_body(ctx, arr, http) < 0) {
        return -1;
    }
    return 0;
}

static inline int mln_lang_http_parse_result_build_type(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_string_t *type)
{
    mln_lang_var_t *_new, *v;
    mln_string_t key = mln_string("type");

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if ((_new = mln_lang_var_create_string(ctx, type, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_request_result_build_method(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_u32_t method = mln_http_method_get(http);
    mln_string_t key = mln_string("method");
    mln_lang_var_t *_new, *v;
    mln_string_t methods[] = {
        mln_string("GET"),
        mln_string("POST"),
        mln_string("HEAD"),
        mln_string("PUT"),
        mln_string("DELETE"),
        mln_string("TRACE"),
        mln_string("CONNECT"),
        mln_string("OPTIONS"),
    };

    if (method > M_HTTP_OPTIONS || method < M_HTTP_GET) {
        mln_lang_errmsg(ctx, "Invalid HTTP request method.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, &methods[method], NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_request_result_build_version(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_u32_t version = mln_http_version_get(http);
    mln_string_t key = mln_string("version");
    mln_lang_var_t *_new, *v;
    mln_string_t versions[] = {
        mln_string("1.0"),
        mln_string("1.1"),
    };

    if (version > M_HTTP_VERSION_1_1 || version < M_HTTP_VERSION_1_0) {
        mln_lang_errmsg(ctx, "Invalid HTTP request version.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, &versions[version], NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_request_result_build_uri(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("uri");
    mln_lang_var_t *_new, *v;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, mln_http_uri_get(http), NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_request_result_build_args(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("args");
    mln_string_t empty = mln_string("");
    mln_string_t *args = mln_http_args_get(http);
    mln_lang_var_t *_new, *v;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if (args == NULL) args = &empty;

    if ((_new = mln_lang_var_create_string(ctx, args, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_result_build_headers(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("headers");
    mln_hash_t *headers = mln_http_header_get(http);
    mln_lang_var_t *_new, *v;
    mln_lang_array_t *a;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_array(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    a = mln_lang_var_val_get(v)->data.array;

    if (mln_hash_iterate(headers, mln_lang_http_parse_result_build_headers_iterate, a) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_http_parse_result_build_headers_iterate(mln_hash_t *h, void *k, void *v, void *udata)
{
    mln_lang_array_t *arr = (mln_lang_array_t *)udata;
    mln_lang_ctx_t *ctx = arr->ctx;
    mln_lang_var_t *_new, *_v;

    if ((_new = mln_lang_var_create_string(ctx, (mln_string_t *)k, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    _v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (_v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, (mln_string_t *)v, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, _v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_result_build_body(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t cl_key = mln_string("Content-Length");
    mln_string_t *cl_val;
    mln_string_t tmp = mln_string("");
    mln_string_t key = mln_string("body");
    mln_lang_var_t *_new, *v;
    mln_sauto_t len = 0;
    mln_buf_t *b = (mln_buf_t *)mln_http_data_get(http);

    cl_val = mln_http_field_get(http, &cl_key);
    if (cl_val == NULL) {
        goto ok;
    }

    len = (mln_sauto_t)atol((char *)(cl_val->data));
    if (len < 0) {
        mln_lang_errmsg(ctx, "Invalid Content-Length");
        return -1;
    }
    if (!len) goto ok;

    mln_string_nset(&tmp, b->left_pos, len);
    b->left_pos += len;

ok:
    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, &tmp, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_response_result_build(mln_lang_ctx_t *ctx, mln_http_t *http, mln_lang_array_t *arr)
{
    mln_string_t type = mln_string("response");

    if (mln_lang_http_parse_result_build_type(ctx, arr, &type) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_response_result_build_version(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_response_result_build_code(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_result_build_headers(ctx, arr, http) < 0) {
        return -1;
    }
    if (mln_lang_http_parse_result_build_body(ctx, arr, http) < 0) {
        return -1;
    }
    return 0;
}

static inline int mln_lang_http_parse_response_result_build_version(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_u32_t version = mln_http_version_get(http);
    mln_string_t key = mln_string("version");
    mln_lang_var_t *_new, *v;
    mln_string_t versions[] = {
        mln_string("1.0"),
        mln_string("1.1"),
    };

    if (version > M_HTTP_VERSION_1_1 || version < M_HTTP_VERSION_1_0) {
        mln_lang_errmsg(ctx, "Invalid HTTP response version.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_string(ctx, &versions[version], NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}

static inline int mln_lang_http_parse_response_result_build_code(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("code");
    mln_lang_var_t *_new, *v;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }

    if ((_new = mln_lang_var_create_int(ctx, mln_http_status_get(http), NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_value_set(ctx, v, _new) < 0) {
        mln_lang_var_free(_new);
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    mln_lang_var_free(_new);
    return 0;
}


static int mln_lang_http_create_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("create");
    mln_string_t v1 = mln_string("data");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_http_create_process, NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v1, M_LANG_VAR_REFER, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        mln_lang_func_detail_free(func);
        return -1;
    }
    mln_lang_func_detail_arg_append(func, var);
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_FUNC, func)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &funcname, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
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

static mln_lang_var_t *mln_lang_http_create_process(mln_lang_ctx_t *ctx)
{
    mln_http_t *http;
    mln_tcp_conn_t conn;
    mln_lang_array_t *a;
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *var, *ret_var = NULL;
    mln_string_t v1 = mln_string("data");
    mln_string_t *ret;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    var = sym->data.var;
    if (mln_lang_var_val_type_get(var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid argument 1.");
        return NULL;
    }
    if ((a = mln_lang_var_val_get(var)->data.array) == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument 1.");
        return NULL;
    }
    if (!mln_rbtree_node_num(a->elems_index)) {
        if ((ret_var = mln_lang_var_create_nil(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }

    if (mln_tcp_conn_init(&conn, -1) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    if ((http = mln_http_init(&conn, a, mln_lang_http_create_process_body_handler)) == NULL) {
        mln_tcp_conn_destroy(&conn);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    ret = mln_lang_http_create_process_generate(ctx, a, http);
    mln_http_destroy(http);
    mln_tcp_conn_destroy(&conn);
    if (ret == NULL) {
        return NULL;
    }

    ret_var = mln_lang_var_create_ref_string(ctx, ret, NULL);
    mln_string_free(ret);
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    return ret_var;
}

static int mln_lang_http_create_process_body_handler(mln_http_t *http, mln_chain_t **head, mln_chain_t **tail)
{
    mln_lang_array_t *arr = mln_http_data_get(http);
    mln_string_t key = mln_string("body");
    mln_string_t cl_key = mln_string("Content-Length");
    mln_string_t tmp;
    mln_lang_var_t *_new, *v;
    mln_string_t *s;
    mln_alloc_t *pool = mln_http_pool_get(http);
    mln_chain_t *c;
    mln_buf_t *b;
    mln_u8_t cl_buf[128];
    mln_lang_ctx_t *ctx = arr->ctx;
    int n;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return M_HTTP_RET_ERROR;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return M_HTTP_RET_ERROR;
    }
    if (mln_lang_var_val_type_get(v) == M_LANG_VAL_TYPE_NIL) {
        return M_HTTP_RET_DONE;
    }
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "'body' should be a string.");
        return M_HTTP_RET_ERROR;
    }
    s = mln_lang_var_val_get(v)->data.s;
    if (!s->len) return M_HTTP_RET_DONE;

    if (mln_http_field_get(http, &cl_key) == NULL) {
#if defined(WIN32) && defined(__pentiumpro__)
        n = snprintf((char *)cl_buf, sizeof(cl_buf) - 1, "%I64u", s->len);
#elif defined(WIN32) || defined(i386) || defined(__arm__) || defined(__wasm__)
        n = snprintf((char *)cl_buf, sizeof(cl_buf) - 1, "%llu", s->len);
#else
        n = snprintf((char *)cl_buf, sizeof(cl_buf) - 1, "%lu", s->len);
#endif
        cl_buf[n] = 0;
        mln_string_nset(&tmp, cl_buf, n);
        if (mln_http_field_set(http, &cl_key, &tmp) < 0) {
            mln_lang_errmsg(ctx, "No memory.");
            return M_HTTP_RET_ERROR;
        }
    }

    if ((c = mln_chain_new(pool)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return M_HTTP_RET_ERROR;
    }
    if ((b = mln_buf_new(pool)) == NULL) {
        mln_chain_pool_release(c);
        mln_lang_errmsg(ctx, "No memory.");
        return M_HTTP_RET_ERROR;
    }
    c->buf = b;
    b->pos = b->left_pos = b->start = s->data;
    b->last = b->end = s->data + s->len;
    b->temporary = 1;

    mln_chain_add(head, tail, c);

    return M_HTTP_RET_DONE;
}

static inline mln_string_t *mln_lang_http_create_process_generate(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("type");
    mln_string_t req = mln_string("request");
    mln_string_t resp = mln_string("response");
    mln_lang_var_t *_new, *v;
    mln_chain_t *head = NULL, *tail = NULL;
    mln_string_t *s;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "'type' should be a string.");
        return NULL;
    }
    s = mln_lang_var_val_get(v)->data.s;

    if (!mln_string_strcmp(s, &req)) {
        mln_http_type_set(http, M_HTTP_REQUEST);
        if (mln_lang_http_create_process_generate_request_method(ctx, arr, http) < 0) return NULL;
        if (mln_lang_http_create_process_generate_version(ctx, arr, http) < 0) return NULL;
        if (mln_lang_http_create_process_generate_request_uri(ctx, arr, http) < 0) return NULL;
        if (mln_lang_http_create_process_generate_request_args(ctx, arr, http) < 0) return NULL;
        if (mln_lang_http_create_process_generate_headers(ctx, arr, http) < 0) return NULL;
        if (mln_http_generate(http, &head, &tail) != M_HTTP_RET_DONE) return NULL;
        s = mln_lang_http_create_process_generate_string(ctx, head);
        mln_chain_pool_release(head);
    } else if (!mln_string_strcmp(s, &resp)) {
        mln_http_type_set(http, M_HTTP_RESPONSE);
        if (mln_lang_http_create_process_generate_version(ctx, arr, http) < 0) return NULL;
        if (mln_lang_http_create_process_generate_response_code(ctx, arr, http) < 0) return NULL;
        if (mln_lang_http_create_process_generate_headers(ctx, arr, http) < 0) return NULL;
        if (mln_http_generate(http, &head, &tail) != M_HTTP_RET_DONE) return NULL;
        s = mln_lang_http_create_process_generate_string(ctx, head);
        mln_chain_pool_release(head);
    } else {
        mln_lang_errmsg(ctx, "Invalid value indexed by 'type' should be 'request' or 'response'.");
        s = NULL;
    }

    return s;
}

static inline int mln_lang_http_create_process_generate_request_method(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("method");
    mln_lang_var_t *_new, *v;
    mln_string_t methods[] = {
        mln_string("GET"),
        mln_string("POST"),
        mln_string("HEAD"),
        mln_string("PUT"),
        mln_string("DELETE"),
        mln_string("TRACE"),
        mln_string("CONNECT"),
        mln_string("OPTIONS"),
    };
    mln_string_t *s;
    int i;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "'method' should be a string.");
        return -1;
    }
    s = mln_lang_var_val_get(v)->data.s;

    for (i = 0; i < sizeof(methods)/sizeof(mln_string_t); ++i) {
        if (!mln_string_strcmp(s, &methods[i])) {
            break;
        }
    }
    if (i >= sizeof(methods)/sizeof(mln_string_t)) {
        mln_lang_errmsg(ctx, "Invalid method.");
        return -1;
    }
    mln_http_method_set(http, i);

    return 0;
}

static inline int mln_lang_http_create_process_generate_version(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("version");
    mln_lang_var_t *_new, *v;
    mln_string_t versions[] = {
        mln_string("1.0"),
        mln_string("1.1"),
    };
    mln_string_t *s;
    int i;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "'version' should be a string.");
        return -1;
    }
    s = mln_lang_var_val_get(v)->data.s;

    for (i = 0; i < sizeof(versions)/sizeof(mln_string_t); ++i) {
        if (!mln_string_strcmp(s, &versions[i])) {
            break;
        }
    }
    if (i >= sizeof(versions)/sizeof(mln_string_t)) {
        mln_lang_errmsg(ctx, "Invalid version.");
        return -1;
    }
    mln_http_version_set(http, i);

    return 0;
}

static inline int mln_lang_http_create_process_generate_request_uri(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("uri");
    mln_lang_var_t *_new, *v;
    mln_string_t *s;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "'uri' should be a string.");
        return -1;
    }
    s = mln_lang_var_val_get(v)->data.s;

    mln_http_uri_set(http, mln_string_ref(s));

    return 0;
}

static inline int mln_lang_http_create_process_generate_request_args(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("args");
    mln_lang_var_t *_new, *v;
    mln_string_t *s;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "'args' should be a string.");
        return -1;
    }
    s = mln_lang_var_val_get(v)->data.s;

    mln_http_args_set(http, mln_string_ref(s));

    return 0;
}

static inline int mln_lang_http_create_process_generate_headers(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("headers");
    mln_lang_var_t *_new, *v;
    mln_lang_array_t *a;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "'headers' should be an array.");
        return -1;
    }
    a = mln_lang_var_val_get(v)->data.array;

    if (mln_rbtree_iterate(a->elems_key, mln_lang_http_create_process_generate_headers_iterate, http) < 0)
        return -1;

    return 0;
}

static int mln_lang_http_create_process_generate_headers_iterate(mln_rbtree_node_t *node, void *udata)
{
    mln_http_t *http = (mln_http_t *)udata;
    mln_lang_array_elem_t *e = (mln_lang_array_elem_t *)mln_rbtree_node_data_get(node);
    mln_lang_ctx_t *ctx = ((mln_lang_array_t *)mln_http_data_get(http))->ctx;

    if (mln_lang_var_val_type_get(e->key) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Header's key should be a string.");
        return -1;
    }
    if (mln_lang_var_val_type_get(e->value) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Header's value should be a string.");
        return -1;
    }

    if (mln_http_field_set(http, mln_lang_var_val_get(e->key)->data.s, mln_lang_var_val_get(e->value)->data.s) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    return 0;
}

static inline int mln_lang_http_create_process_generate_response_code(mln_lang_ctx_t *ctx, mln_lang_array_t *arr, mln_http_t *http)
{
    mln_string_t key = mln_string("code");
    mln_lang_var_t *_new, *v;

    if ((_new = mln_lang_var_create_string(ctx, &key, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    v = mln_lang_array_get(ctx, arr, _new);
    mln_lang_var_free(_new);
    if (v == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_var_val_type_get(v) == M_LANG_VAL_TYPE_NIL) {
        mln_http_status_set(http, 200);
    } else if (mln_lang_var_val_type_get(v) == M_LANG_VAL_TYPE_INT) {
        mln_http_status_set(http, mln_lang_var_val_get(v)->data.i);
    } else {
        mln_lang_errmsg(ctx, "'code' should be an integer.");
        return -1;
    }

    return 0;
}

static inline mln_string_t *mln_lang_http_create_process_generate_string(mln_lang_ctx_t *ctx, mln_chain_t *in)
{
    mln_size_t size = 0;
    mln_chain_t *c;
    mln_u8ptr_t p;
    mln_string_t tmp, *s;

    for (c = in; c != NULL; c = c->next) {
        size += mln_buf_size(c->buf);
    }

    mln_string_nset(&tmp, NULL, size);
    if ((s = mln_string_pool_dup(ctx->pool, &tmp)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    p = s->data;
    for (c = in; c != NULL; c = c->next) {
        memcpy(p, c->buf->pos, mln_buf_size(c->buf));
        p += mln_buf_size(c->buf);
    }

    return s;
}

#if defined(MLN_TLS)
/* ======================================================================== *
 *  http.https_request(args) -- end-to-end HTTPS client                     *
 *                                                                          *
 *  Args dictionary:                                                        *
 *    host         (string)   required.  Server name to connect to.         *
 *    service      (string)   required.  Port or service ("443", "https").  *
 *    request      (array)    required.  Same shape as http.create input.   *
 *    timeout      (int|nil)  optional.  Per-step timeout in ms.            *
 *    ca           (string)   optional.  Path to a CA bundle (PEM).         *
 *    verify       (bool)     optional.  Verify peer when CA is set.        *
 *    sni          (string)   optional.  SNI hostname (defaults to host).   *
 *    verify_host  (string)   optional.  Hostname to verify in cert.        *
 *    conf         (int)      optional.  Pre-built conf from tls.conf_new.  *
 *                                                                          *
 *  Returns the parsed response dictionary on success, false on error,      *
 *  nil on timeout.  The script suspends for the duration of the entire     *
 *  exchange; other coroutines on the same lang scheduler keep running.     *
 *                                                                          *
 *  The whole exchange runs in one heap-allocated state struct.  Every      *
 *  termination edge -- success, error, timeout, OOM, premature script      *
 *  teardown via the per-ctx resource list -- routes through                *
 *  mln_lang_https_state_free() so cleanup happens exactly once.            *
 * ======================================================================== */

enum {
    MLN_LANG_HTTPS_STATE_CONNECT = 0,
    MLN_LANG_HTTPS_STATE_HANDSHAKE,
    MLN_LANG_HTTPS_STATE_SEND,
    MLN_LANG_HTTPS_STATE_RECV
};

#define MLN_LANG_HTTPS_RETRY_MAX 64
#define MLN_LANG_HTTPS_HOST_MAX  256
#define MLN_LANG_HTTPS_SVC_MAX   64

typedef struct mln_lang_https_state_s {
    mln_lang_t            *lang;
    /*
     * The lang ctx the request originated from.  Set unconditionally
     * at state allocation -- handlers need it to install the return
     * variable and resume the script.  Distinct from the "linked"
     * bit below: ctx being non-NULL does NOT imply the state is
     * already linked into the per-ctx chain; both pieces of state are
     * tracked independently so a fail-before-suspend path can run
     * state_free without corrupting a chain it never joined.
     */
    mln_lang_ctx_t        *ctx;
    int                    fd;
    int                    state;
    int                    timeout;
    int                    retry;
    int                    have_conn;     /* mln_tcp_conn_init succeeded */
    int                    own_conf;      /* we created the SSL_CTX wrapper */
    int                    linked;        /* on per-ctx https chain */
    int                    request_built; /* request chain appended to send queue */
    mln_tcp_conn_t         conn;
    mln_tcp_tls_conf_t    *conf;
    mln_http_t            *http;
    mln_lang_array_t      *request;   /* not owned; lives in suspended ctx */
    /* Hostnames are copied to heap so any reshuffling of the ctx pool
     * during the suspend window can't leave us with dangling pointers. */
    mln_u8_t              *sni_buf;
    mln_size_t             sni_len;
    mln_u8_t              *vhost_buf;
    mln_size_t             vhost_len;
    struct mln_lang_https_state_s *prev;
    struct mln_lang_https_state_s *next;
} mln_lang_https_state_t;

typedef struct mln_lang_ctx_https_s {
    mln_lang_ctx_t            *ctx;
    mln_lang_https_state_t    *head;
    mln_lang_https_state_t    *tail;
} mln_lang_ctx_https_t;

MLN_CHAIN_FUNC_DECLARE(static inline, mln_lang_https, mln_lang_https_state_t,);
MLN_CHAIN_FUNC_DEFINE (static inline, mln_lang_https, mln_lang_https_state_t, prev, next);

static mln_lang_var_t *mln_lang_https_request_process(mln_lang_ctx_t *ctx);
static void mln_lang_https_state_free(mln_lang_https_state_t *st);
static void mln_lang_https_resume_with(mln_lang_https_state_t *st, mln_lang_var_t *ret);
static int  mln_lang_https_build_request(mln_lang_https_state_t *st);
static int  mln_lang_https_drive(mln_lang_https_state_t *st);
static int  mln_lang_https_advance_send(mln_lang_https_state_t *st);
static int  mln_lang_https_advance_recv(mln_lang_https_state_t *st);
static int  mln_lang_https_arm(mln_lang_https_state_t *st, mln_u32_t fallback);
static void mln_lang_https_connect_handler(mln_event_t *ev, int fd, void *data);
static void mln_lang_https_handler        (mln_event_t *ev, int fd, void *data);
static void mln_lang_https_timeout_handler(mln_event_t *ev, int fd, void *data);
static void mln_lang_ctx_https_free(mln_lang_ctx_https_t *lch);

/* mln_lang_array_get with a C-string key.  Returns the stored value's
 * wrapper var on success (caller reads val/type/data) or NULL on
 * memory failure or absence. */
static mln_lang_var_t *
mln_lang_https_arr_get(mln_lang_ctx_t *ctx, mln_lang_array_t *a, const char *key)
{
    mln_string_t k;
    mln_lang_var_t *kv, *v;
    mln_string_nset(&k, (char *)key, strlen(key));
    if ((kv = mln_lang_var_create_string(ctx, &k, NULL)) == NULL) return NULL;
    v = mln_lang_array_get(ctx, a, kv);
    mln_lang_var_free(kv);
    return v;
}

static int mln_lang_http_https_request_register(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("https_request");
    mln_string_t a1       = mln_string("args");

    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL,
                                         mln_lang_https_request_process,
                                         NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &a1, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        mln_lang_func_detail_free(func);
        return -1;
    }
    mln_lang_func_detail_arg_append(func, var);

    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_FUNC, func)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &funcname, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        return -1;
    }
    if (mln_lang_set_member_add(ctx->pool, obj->members, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }

    /* Per-ctx resource: chain of in-flight states so a coroutine
     * teardown frees each one and disarms its event registration. */
    {
        mln_lang_ctx_https_t *lch;
        if ((lch = mln_lang_ctx_resource_fetch(ctx, "https")) == NULL) {
            if ((lch = (mln_lang_ctx_https_t *)mln_alloc_m(ctx->pool, sizeof(*lch))) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return -1;
            }
            lch->ctx  = ctx;
            lch->head = lch->tail = NULL;
            if (mln_lang_ctx_resource_register(ctx, "https", lch,
                                               (mln_lang_resource_free)mln_lang_ctx_https_free) < 0) {
                mln_alloc_free(lch);
                mln_lang_errmsg(ctx, "No memory.");
                return -1;
            }
        }
    }
    return 0;
}

static void mln_lang_ctx_https_free(mln_lang_ctx_https_t *lch)
{
    if (lch == NULL) return;
    mln_lang_https_state_t *st;
    while ((st = lch->head) != NULL) {
        mln_lang_https_chain_del(&lch->head, &lch->tail, st);
        /* Unhook so state_free does not try to re-walk the chain we
         * are currently draining; the ctx pointer is also reset so
         * resume helpers can no-op against a teardown. */
        st->linked = 0;
        st->ctx = NULL;
        mln_lang_https_state_free(st);
    }
    mln_alloc_free(lch);
}

static void mln_lang_https_state_free(mln_lang_https_state_t *st)
{
    if (st == NULL) return;
    /* Only call chain_del when the state is actually linked: the
     * MLN_CHAIN macros assume the node is in the list and would
     * otherwise clobber head/tail to NULL even if other states are
     * in the chain. */
    if (st->linked && st->ctx != NULL) {
        mln_lang_ctx_https_t *lch = mln_lang_ctx_resource_fetch(st->ctx, "https");
        if (lch != NULL) {
            mln_lang_https_chain_del(&lch->head, &lch->tail, st);
        }
        st->linked = 0;
    }
    st->ctx = NULL;
    if (st->fd >= 0) {
        if (st->lang && st->lang->ev) {
            mln_event_fd_set(st->lang->ev, st->fd, M_EV_CLR,
                             M_EV_UNLIMITED, NULL, NULL);
        }
        mln_socket_close(st->fd);
        st->fd = -1;
    }
    if (st->http != NULL) { mln_http_destroy(st->http); st->http = NULL; }
    if (st->have_conn) {
        mln_tcp_conn_destroy(&st->conn);
        st->have_conn = 0;
    }
    if (st->own_conf && st->conf != NULL) {
        mln_tcp_tls_conf_free(st->conf);
    }
    st->conf = NULL;
    free(st->sni_buf);
    free(st->vhost_buf);
    free(st);
}

/* Install ret on the script ctx, resume the ctx, free the state.
 * Must be called exactly once per state, under the lang mutex. */
static void mln_lang_https_resume_with(mln_lang_https_state_t *st, mln_lang_var_t *ret)
{
    mln_lang_ctx_t *ctx = st->ctx;
    if (ret != NULL && ctx != NULL) {
        mln_lang_ctx_set_ret_var(ctx, ret);
    }
    if (ctx != NULL) {
        mln_lang_ctx_continue(ctx);
    }
    mln_lang_https_state_free(st);
}

/* Serialize the request dict into a single send-queue chain.  Must run
 * after mln_tcp_conn_tls_init has rebuilt the conn pool (because
 * tls_init internally calls mln_tcp_conn_init, which destroys the
 * previous pool and any chains attached to it). */
static int mln_lang_https_build_request(mln_lang_https_state_t *st)
{
    mln_string_t *s;
    mln_alloc_t  *pool;
    mln_chain_t  *c;
    mln_buf_t    *b;
    mln_u8ptr_t   buf;
    mln_size_t    len;

    if (st->request_built) return 0;
    if ((st->http = mln_http_init(&st->conn, st->request,
                                  mln_lang_http_create_process_body_handler)) == NULL) {
        return -1;
    }
    s = mln_lang_http_create_process_generate(st->ctx, st->request, st->http);
    if (s == NULL) {
        mln_http_destroy(st->http);
        st->http = NULL;
        return -1;
    }
    len  = s->len;
    pool = mln_tcp_conn_pool_get(&st->conn);
    if ((c = mln_chain_new(pool)) == NULL) goto nomem;
    if ((b = mln_buf_new(pool)) == NULL) {
        mln_chain_pool_release(c);
        goto nomem;
    }
    if ((buf = (mln_u8ptr_t)mln_alloc_m(pool, len ? len : 1)) == NULL) {
        mln_chain_pool_release(c);
        goto nomem;
    }
    if (len) memcpy(buf, s->data, len);
    mln_string_free(s);

    c->buf = b;
    b->left_pos      = buf;
    b->pos           = buf;
    b->start         = buf;
    b->last          = buf + len;
    b->end           = buf + len;
    b->in_memory     = 1;
    b->last_buf      = 1;
    b->last_in_chain = 1;
    /* Reset the parser state so we can reuse the same mln_http_t for
     * parsing the response after we're done sending the request. */
    mln_http_reset(st->http);
    mln_tcp_conn_append(&st->conn, c, M_C_SEND);
    st->request_built = 1;
    return 0;

nomem:
    mln_string_free(s);
    mln_http_destroy(st->http);
    st->http = NULL;
    return -1;
}

/* Re-arm the fd registration honouring TLS want flags. */
static int mln_lang_https_arm(mln_lang_https_state_t *st, mln_u32_t fallback)
{
    mln_u32_t flag = 0;
    if (mln_tcp_conn_tls_want_read(&st->conn))  flag |= M_EV_RECV;
    if (mln_tcp_conn_tls_want_write(&st->conn)) flag |= M_EV_SEND;
    if (flag == 0) flag = fallback;
    if (mln_event_fd_set(st->lang->ev, st->fd,
                         flag|M_EV_NONBLOCK|M_EV_ONESHOT,
                         st->timeout, st,
                         mln_lang_https_handler) < 0)
        return -1;
    if (st->timeout != M_EV_UNLIMITED) {
        mln_event_fd_timeout_handler_set(st->lang->ev, st->fd, st,
                                         mln_lang_https_timeout_handler);
    }
    return 0;
}

static int mln_lang_https_advance_send(mln_lang_https_state_t *st)
{
    int r = mln_tcp_conn_send(&st->conn);
    mln_chain_pool_release_all(mln_tcp_conn_remove(&st->conn, M_C_SENT));
    if (r == M_C_ERROR || r == M_C_CLOSED) return -1;
    if (mln_tcp_conn_head(&st->conn, M_C_SEND) == NULL
        && !mln_tcp_conn_tls_want_read(&st->conn)
        && !mln_tcp_conn_tls_want_write(&st->conn))
    {
        st->state = MLN_LANG_HTTPS_STATE_RECV;
        return mln_lang_https_arm(st, M_EV_RECV);
    }
    return mln_lang_https_arm(st, M_EV_SEND);
}

static int mln_lang_https_advance_recv(mln_lang_https_state_t *st)
{
    int r = mln_tcp_conn_recv(&st->conn, M_C_TYPE_MEMORY);
    if (r == M_C_ERROR) return -1;

    mln_chain_t *in = mln_tcp_conn_remove(&st->conn, M_C_RECV);
    if (in != NULL) {
        int pr = mln_http_parse(st->http, &in);
        if (pr == M_HTTP_RET_DONE) {
            mln_lang_var_t *ret_var = mln_lang_http_parse_result_build(st->ctx, st->http);
            mln_chain_pool_release_all(in);
            if (ret_var == NULL) return -1;
            mln_lang_https_resume_with(st, ret_var);
            return 1;
        }
        if (pr == M_HTTP_RET_ERROR) {
            mln_chain_pool_release_all(in);
            return -1;
        }
        /* M_HTTP_RET_OK -- need more bytes.  Free fully-consumed chains
         * but put any partially-consumed tail back on the recv queue so
         * subsequent reads append after it. */
        mln_chain_t *rem = in;
        while (rem != NULL && rem->buf != NULL && mln_buf_left_size(rem->buf) == 0) {
            mln_chain_t *tmp = rem;
            rem = rem->next;
            tmp->next = NULL;
            mln_chain_pool_release(tmp);
        }
        if (rem != NULL)
            mln_tcp_conn_append_chain(&st->conn, rem, NULL, M_C_RECV);
    }
    if (r == M_C_CLOSED && mln_tcp_conn_head(&st->conn, M_C_RECV) == NULL) {
        return -1;
    }
    return mln_lang_https_arm(st, M_EV_RECV);
}

static int mln_lang_https_drive(mln_lang_https_state_t *st)
{
    int r;
    switch (st->state) {
    case MLN_LANG_HTTPS_STATE_HANDSHAKE:
        r = mln_tcp_conn_tls_handshake(&st->conn);
        if (r == M_C_ERROR || r == M_C_CLOSED) return -1;
        if (r == M_C_FINISH) {
            if (mln_lang_https_build_request(st) < 0) return -1;
            st->state = MLN_LANG_HTTPS_STATE_SEND;
            return mln_lang_https_advance_send(st);
        }
        return mln_lang_https_arm(st, M_EV_RECV);
    case MLN_LANG_HTTPS_STATE_SEND:
        return mln_lang_https_advance_send(st);
    case MLN_LANG_HTTPS_STATE_RECV:
        return mln_lang_https_advance_recv(st);
    default:
        return -1;
    }
}

static void mln_lang_https_connect_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_https_state_t *st = (mln_lang_https_state_t *)data;
    mln_lang_t *lang = st->lang;
    int err = 0;
    socklen_t errlen = sizeof(err);

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
    if (err == EINPROGRESS && (++st->retry <= MLN_LANG_HTTPS_RETRY_MAX)) {
        mln_event_fd_set(ev, fd,
                         M_EV_SEND|M_EV_ERROR|M_EV_NONBLOCK|M_EV_ONESHOT,
                         st->timeout, st,
                         mln_lang_https_connect_handler);
        if (st->timeout != M_EV_UNLIMITED) {
            mln_event_fd_timeout_handler_set(ev, fd, st,
                                             mln_lang_https_timeout_handler);
        }
        mln_lang_mutex_unlock(lang);
        return;
    }
    if (err != 0) {
        mln_lang_https_resume_with(st, NULL);
        mln_lang_mutex_unlock(lang);
        return;
    }

    /* TCP layer is up; upgrade to TLS.  tls_init internally reinitializes
     * the tcp conn (creating a fresh pool), so any chains attached to a
     * previous pool would be discarded -- we deliberately attach none. */
    if (mln_tcp_conn_tls_init(&st->conn, fd, st->conf) < 0) {
        /* On failure, tls_init has already destroyed the pool and set
         * it to NULL.  Mark have_conn=0 so we don't double-destroy and
         * leave the fd cleanup to the state free. */
        st->have_conn = 0;
        mln_lang_https_resume_with(st, NULL);
        mln_lang_mutex_unlock(lang);
        return;
    }
    /* After successful tls_init the conn is fully owned by st->conn,
     * but the fd is now in st->conn too -- ownership is shared between
     * st->fd (raw close path) and the conn (destroy releases pool only).
     * close happens via mln_socket_close on st->fd in state_free. */
    st->have_conn = 1;
    mln_tcp_conn_set_nonblock(&st->conn, 1);

    if (st->sni_buf != NULL && st->sni_len > 0) {
        mln_string_t s;
        mln_string_nset(&s, st->sni_buf, st->sni_len);
        if (mln_tcp_conn_tls_set_sni(&st->conn, &s) < 0) {
            mln_lang_https_resume_with(st, NULL);
            mln_lang_mutex_unlock(lang);
            return;
        }
    }
    if (st->vhost_buf != NULL && st->vhost_len > 0) {
        mln_string_t s;
        mln_string_nset(&s, st->vhost_buf, st->vhost_len);
        if (mln_tcp_conn_tls_set_verify_host(&st->conn, &s) < 0) {
            mln_lang_https_resume_with(st, NULL);
            mln_lang_mutex_unlock(lang);
            return;
        }
    }

    st->state = MLN_LANG_HTTPS_STATE_HANDSHAKE;
    if (mln_lang_https_drive(st) < 0) {
        mln_lang_https_resume_with(st, NULL);
    }
    mln_lang_mutex_unlock(lang);
}

static void mln_lang_https_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_https_state_t *st = (mln_lang_https_state_t *)data;
    mln_lang_t *lang = st->lang;
    (void)ev; (void)fd;
    mln_lang_mutex_lock(lang);
    if (mln_lang_https_drive(st) < 0) {
        mln_lang_https_resume_with(st, NULL);
    }
    mln_lang_mutex_unlock(lang);
}

static void mln_lang_https_timeout_handler(mln_event_t *ev, int fd, void *data)
{
    mln_lang_https_state_t *st = (mln_lang_https_state_t *)data;
    mln_lang_t *lang = st->lang;
    mln_lang_var_t *ret_var;
    (void)ev; (void)fd;
    mln_lang_mutex_lock(lang);
    ret_var = mln_lang_var_create_nil(st->ctx, NULL);
    mln_lang_https_resume_with(st, ret_var);
    mln_lang_mutex_unlock(lang);
}

/* Copy hostname/verify-host strings to heap so they survive the
 * suspend window without depending on ctx pool churn. */
static int mln_lang_https_dup_to(mln_u8_t **dst, mln_size_t *len,
                                 const mln_string_t *src)
{
    *len = src->len;
    if (*len == 0) { *dst = NULL; return 0; }
    *dst = (mln_u8_t *)malloc(*len);
    if (*dst == NULL) return -1;
    memcpy(*dst, src->data, *len);
    return 0;
}

static mln_lang_var_t *mln_lang_https_request_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v_args = mln_string("args");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *ret_var = NULL, *v;
    mln_lang_array_t *a;
    mln_lang_https_state_t *st = NULL;
    mln_lang_ctx_https_t *lch;
    char host[MLN_LANG_HTTPS_HOST_MAX] = {0};
    char service[MLN_LANG_HTTPS_SVC_MAX] = {0};
    struct addrinfo hints, *res = NULL;
    int fd = -1;
    mln_string_t *sni = NULL, *vhost = NULL, *ca = NULL;
    mln_s64_t conf_id = 0;
    int has_conf = 0;
    mln_u32_t verify = 0;

    if ((sym = mln_lang_symbol_node_search(ctx, &v_args, 1)) == NULL ||
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY)
    {
        mln_lang_errmsg(ctx, "Argument 'args' must be an array.");
        return NULL;
    }
    a = sym->data.var->val->data.array;

    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    if ((st = (mln_lang_https_state_t *)calloc(1, sizeof(*st))) == NULL) {
        mln_lang_var_free(ret_var);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    st->lang    = ctx->lang;
    st->ctx     = ctx;
    st->fd      = -1;
    st->state   = MLN_LANG_HTTPS_STATE_CONNECT;
    st->timeout = M_EV_UNLIMITED;
    st->request = NULL;

    /* host */
    if ((v = mln_lang_https_arr_get(ctx, a, "host")) == NULL ||
        mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING)
    {
        mln_lang_errmsg(ctx, "args.host must be a string.");
        goto fail_arg;
    }
    if (v->val->data.s->len >= sizeof(host)) {
        mln_lang_errmsg(ctx, "args.host too long.");
        goto fail_arg;
    }
    memcpy(host, v->val->data.s->data, v->val->data.s->len);

    /* service */
    if ((v = mln_lang_https_arr_get(ctx, a, "service")) == NULL ||
        mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_STRING)
    {
        mln_lang_errmsg(ctx, "args.service must be a string.");
        goto fail_arg;
    }
    if (v->val->data.s->len >= sizeof(service)) {
        mln_lang_errmsg(ctx, "args.service too long.");
        goto fail_arg;
    }
    memcpy(service, v->val->data.s->data, v->val->data.s->len);

    /* request */
    if ((v = mln_lang_https_arr_get(ctx, a, "request")) == NULL ||
        mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_ARRAY)
    {
        mln_lang_errmsg(ctx, "args.request must be an array.");
        goto fail_arg;
    }
    st->request = v->val->data.array;

    /* timeout */
    if ((v = mln_lang_https_arr_get(ctx, a, "timeout")) != NULL) {
        mln_u32_t t = mln_lang_var_val_type_get(v);
        if (t == M_LANG_VAL_TYPE_NIL)        st->timeout = M_EV_UNLIMITED;
        else if (t == M_LANG_VAL_TYPE_INT && v->val->data.i >= 0)
                                              st->timeout = (int)v->val->data.i;
        else {
            mln_lang_errmsg(ctx, "args.timeout must be a non-negative int or nil.");
            goto fail_arg;
        }
    }

    /* conf */
    if ((v = mln_lang_https_arr_get(ctx, a, "conf")) != NULL
        && mln_lang_var_val_type_get(v) == M_LANG_VAL_TYPE_INT)
    {
        conf_id = v->val->data.i;
        has_conf = 1;
    }

    if (has_conf) {
        mln_rbtree_t *conf_set = mln_lang_resource_fetch(ctx->lang, "tls_conf");
        if (conf_set != NULL) {
            mln_lang_tls_conf_t tmp;
            mln_rbtree_node_t *rn;
            tmp.id = conf_id; tmp.conf = NULL;
            rn = mln_rbtree_search(conf_set, &tmp);
            if (!mln_rbtree_null(rn, conf_set)) {
                mln_lang_tls_conf_t *node = mln_rbtree_node_data_get(rn);
                st->conf = node->conf;
                st->own_conf = 0;
            }
        }
        if (st->conf == NULL) {
            mln_lang_errmsg(ctx, "args.conf is not a valid TLS conf handle.");
            goto fail_arg;
        }
    } else {
        if ((v = mln_lang_https_arr_get(ctx, a, "ca")) != NULL
            && mln_lang_var_val_type_get(v) == M_LANG_VAL_TYPE_STRING)
        {
            ca = v->val->data.s;
        }
        if ((v = mln_lang_https_arr_get(ctx, a, "verify")) != NULL) {
            mln_u32_t t = mln_lang_var_val_type_get(v);
            if (t == M_LANG_VAL_TYPE_BOOL)      verify = v->val->data.b ? 1 : 0;
            else if (t == M_LANG_VAL_TYPE_INT)  verify = v->val->data.i ? 1 : 0;
        }
        st->conf = mln_tcp_tls_conf_new(M_TLS_CLIENT, NULL, NULL, ca, NULL,
                                        M_TLS_VDEFAULT, verify);
        if (st->conf == NULL) {
            mln_lang_errmsg(ctx, "Failed to build TLS conf.");
            goto fail_arg;
        }
        st->own_conf = 1;
    }

    if ((v = mln_lang_https_arr_get(ctx, a, "sni")) != NULL
        && mln_lang_var_val_type_get(v) == M_LANG_VAL_TYPE_STRING)
    {
        sni = v->val->data.s;
    }
    if ((v = mln_lang_https_arr_get(ctx, a, "verify_host")) != NULL
        && mln_lang_var_val_type_get(v) == M_LANG_VAL_TYPE_STRING)
    {
        vhost = v->val->data.s;
    }
    if (sni != NULL && mln_lang_https_dup_to(&st->sni_buf, &st->sni_len, sni) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        goto fail_arg;
    }
    if (vhost != NULL && mln_lang_https_dup_to(&st->vhost_buf, &st->vhost_len, vhost) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        goto fail_arg;
    }

    /* DNS + socket + connect. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;
    if (getaddrinfo(host, service, &hints, &res) != 0 || res == NULL) {
        mln_lang_https_state_free(st);
        return ret_var; /* false */
    }
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        freeaddrinfo(res);
        mln_lang_https_state_free(st);
        return ret_var;
    }
    st->fd = fd;

    if (mln_event_fd_set(ctx->lang->ev, fd,
                         M_EV_SEND|M_EV_ERROR|M_EV_NONBLOCK|M_EV_ONESHOT,
                         st->timeout, st,
                         mln_lang_https_connect_handler) < 0) {
        freeaddrinfo(res);
        mln_lang_https_state_free(st);
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
#if defined(WIN32)
    if (connect(fd, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR
        && WSAGetLastError() != WSAEWOULDBLOCK) {
#else
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0 && errno != EINPROGRESS) {
#endif
        freeaddrinfo(res);
        mln_lang_https_state_free(st);
        return ret_var;
    }
    freeaddrinfo(res);
    if (st->timeout != M_EV_UNLIMITED) {
        mln_event_fd_timeout_handler_set(ctx->lang->ev, fd, st,
                                         mln_lang_https_timeout_handler);
    }

    /* Hook into per-ctx chain so coroutine teardown finds us. */
    lch = mln_lang_ctx_resource_fetch(ctx, "https");
    if (lch == NULL) {
        mln_lang_https_state_free(st);
        mln_lang_errmsg(ctx, "https ctx resource missing.");
        return NULL;
    }
    mln_lang_https_chain_add(&lch->head, &lch->tail, st);
    st->linked = 1;

    mln_lang_ctx_suspend(ctx);
    return ret_var;

fail_arg:
    mln_lang_https_state_free(st);
    mln_lang_var_free(ret_var);
    return NULL;
}
#endif /* MLN_TLS */

