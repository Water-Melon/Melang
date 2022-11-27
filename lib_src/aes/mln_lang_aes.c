
/*
 * Copyright (C) Niklaus F.Schen.
 */
#include "mln_lang_aes.h"
#include "mln_aes.h"
#include "mln_core.h"
#include "mln_conf.h"

#ifdef __DEBUG__
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x);
#endif

static int melon_init_flag = 0;

static int mln_lang_aes(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static mln_lang_var_t *mln_lang_aes_process(mln_lang_ctx_t *ctx);

static int mln_lang_aes_global_init(void)
{
    mln_conf_t *cf;
    mln_conf_domain_t *cd;

    cf = mln_get_conf();
    if (cf == NULL) return 0;
    cd = cf->search(cf, "main");
    if (cd == NULL) return 0;
    cd->remove(cd, "trace_mode");
    cd->remove(cd, "framework");

    return 0;
}

mln_lang_var_t *init(mln_lang_ctx_t *ctx)
{
    if (!melon_init_flag) {
        struct mln_core_attr cattr;
        cattr.argc = 0;
        cattr.argv = NULL;
        cattr.global_init = mln_lang_aes_global_init;
#if !defined(WINNT)
        cattr.master_process = NULL;
        cattr.worker_process = NULL;
#endif
        if (mln_core_init(&cattr) < 0) {
            fprintf(stderr, "framework init failed.\n");
            return NULL;
        }
        melon_init_flag = 1;
    }
    mln_lang_var_t *obj = mln_lang_var_create_obj(ctx, NULL, NULL);
    if (obj == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (mln_lang_aes(ctx, mln_lang_var_val_get(obj)->data.obj) < 0) {
        mln_lang_var_free(obj);
        return NULL;
    }
    return obj;
}

static int mln_lang_aes(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("aes");
    mln_string_t v1 = mln_string("data");
    mln_string_t v2 = mln_string("key");
    mln_string_t v3 = mln_string("bits");
    mln_string_t v4 = mln_string("op");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_aes_process, NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v1, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        mln_lang_func_detail_free(func);
        return -1;
    }
    mln_lang_func_detail_arg_append(func, var);
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v2, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        mln_lang_func_detail_free(func);
        return -1;
    }
    mln_lang_func_detail_arg_append(func, var);
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v3, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        mln_lang_func_detail_free(func);
        return -1;
    }
    mln_lang_func_detail_arg_append(func, var);
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v4, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
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

static mln_lang_var_t *mln_lang_aes_process(mln_lang_ctx_t *ctx)
{
    mln_u32_t bits = M_AES_128;
    mln_s32_t type, encode = 0;
    mln_lang_val_t *val;
    mln_string_t *tmp;
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("data");
    mln_string_t v2 = mln_string("key");
    mln_string_t v3 = mln_string("bits");
    mln_string_t v4 = mln_string("op");
    mln_lang_symbol_node_t *sym;
    mln_aes_t aes;
    mln_u8ptr_t p, pend;

    if ((sym = mln_lang_symbol_node_search(ctx, &v4, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    val = sym->data.var->val;
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid argument 4.");
        return NULL;
    }
    if ((tmp = val->data.s) == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument 4.");
        return NULL;
    }
    if (!mln_string_const_strcmp(tmp, "encode")) {
        encode = 1;
    } else if (!mln_string_const_strcmp(tmp, "decode")) {
        /*do nothing, encode = 0*/
    } else {
        mln_lang_errmsg(ctx, "Invalid argument 4.");
        return NULL;
    }

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    val = sym->data.var->val;
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid argument 2.");
        return NULL;
    }
    if ((tmp = val->data.s) == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument 2.");
        return NULL;
    }

    if ((sym = mln_lang_symbol_node_search(ctx, &v3, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    type = mln_lang_var_val_type_get(sym->data.var);
    val = sym->data.var->val;
    if (type != M_LANG_VAL_TYPE_INT) {
        mln_lang_errmsg(ctx, "Invalid argument 3.");
        return NULL;
    }
    switch (val->data.i) {
        case 128:
            if (tmp->len < 16) {
                mln_lang_errmsg(ctx, "Invalid argument 3.");
                return NULL;
            }
            break;
        case 192:
            if (tmp->len < 24) {
                mln_lang_errmsg(ctx, "Invalid argument 3.");
                return NULL;
            }
            bits = M_AES_192;
            break;
        case 256:
            if (tmp->len < 32) {
                mln_lang_errmsg(ctx, "Invalid argument 3.");
                return NULL;
            }
            bits = M_AES_256;
            break;
        default:
            mln_lang_errmsg(ctx, "Invalid argument 3.");
            return NULL;
    }

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    val = sym->data.var->val;
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid argument 1.");
        return NULL;
    }
    if (val->data.s == NULL || val->data.s->len%16) {
        mln_lang_errmsg(ctx, "Invalid argument 1.");
        return NULL;
    }
    (void)mln_aes_init(&aes, tmp->data, bits);
    if (encode) {
        p = val->data.s->data;
        pend = val->data.s->data + val->data.s->len;
        for (; p < pend; p += 16) {
            (void)mln_aes_encrypt(&aes, p);
        }
    } else {
        p = val->data.s->data;
        pend = val->data.s->data + val->data.s->len;
        for (; p < pend; p += 16) {
            (void)mln_aes_decrypt(&aes, p);
        }
    }
    if ((ret_var = mln_lang_var_create_ref_string(ctx, val->data.s, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

