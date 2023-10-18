
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
    mln_string_t *slices, *s, _k, _v;
    mln_lang_var_t *_new, *v;
    mln_lang_array_t *a;
    mln_u8ptr_t p, end;

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

    if (args == NULL || !args->len) return 0;

    if ((slices = mln_string_slice(args, "&\0")) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    for (s = slices; s->len != 0; ++s) {
        for (p = s->data, end = s->data + s->len; p < end; ++p) {
            if (*p == (mln_u8_t)'=') break;
        }
        mln_string_nset(&_k, s->data, p - s->data);
        if (p >= end) {
            mln_string_nset(&_v, "", 0);
        } else {
            mln_string_nset(&_v, ++p, end - p);
        }

        if ((_new = mln_lang_var_create_string(ctx, &_k, NULL)) == NULL) {
            mln_string_slice_free(slices);
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
        v = mln_lang_array_get(ctx, a, _new);
        mln_lang_var_free(_new);
        if (v == NULL) {
            mln_string_slice_free(slices);
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }

        if ((_new = mln_lang_var_create_string(ctx, &_v, NULL)) == NULL) {
            mln_string_slice_free(slices);
            mln_lang_errmsg(ctx, "No memory.");
            return -1;
        }
        if (mln_lang_var_value_set(ctx, v, _new) < 0) {
            mln_lang_var_free(_new);
            mln_string_slice_free(slices);
            mln_log(error, "No memory.\n");
            return -1;
        }
        mln_lang_var_free(_new);
    }
    mln_string_slice_free(slices);
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

