
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
static int mln_lang_http_parse_result_build_headers_iterate(void *k, void *v, void *udata);
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

static int mln_lang_http(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    if (mln_lang_http_parse_handler(ctx, obj) < 0) return -1;
    if (mln_lang_http_create_handler(ctx, obj) < 0) return -1;
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

static int mln_lang_http_parse_result_build_headers_iterate(void *k, void *v, void *udata)
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
#if defined(WIN32)
        n = snprintf((char *)cl_buf, sizeof(cl_buf) - 1, "%I64u", s->len);
#elif defined(i386) || defined(__arm__) || defined(__wasm__)
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
    if (mln_lang_var_val_type_get(v) != M_LANG_VAL_TYPE_INT) {
        mln_lang_errmsg(ctx, "'code' should be an integer.");
        return -1;
    }

    mln_http_status_set(http, mln_lang_var_val_get(v)->data.i);

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

