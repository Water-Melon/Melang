
/*
 * Copyright (C) Niklaus F.Schen.
 */
#include "mln_lang_rc.h"
#include "mln_rc.h"
#include "mln_core.h"

#ifdef __DEBUG__
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x);
#endif

static int melon_init_flag = 0;

int mln_lang_rc(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static mln_lang_var_t *mln_lang_rc4_process(mln_lang_ctx_t *ctx);

mln_lang_var_t *init(mln_lang_ctx_t *ctx)
{
    if (!melon_init_flag) {
        struct mln_core_attr cattr;
        cattr.argc = 0;
        cattr.argv = NULL;
        cattr.global_init = NULL;
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
    if (mln_lang_rc(ctx, mln_lang_var_val_get(obj)->data.obj) < 0) {
        mln_lang_var_free(obj);
        return NULL;
    }
    return obj;
}

int mln_lang_rc(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("rc4");
    mln_string_t v1 = mln_string("data");
    mln_string_t v2 = mln_string("key");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_rc4_process, NULL, NULL)) == NULL) {
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
    mln_lang_var_chain_add(&(func->args_head), &(func->args_tail), var);
    ++func->nargs;
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
    mln_lang_var_chain_add(&(func->args_head), &(func->args_tail), var);
    ++func->nargs;
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

static mln_lang_var_t *mln_lang_rc4_process(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_string_t *tmp;
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("data");
    mln_string_t v2 = mln_string("key");
    mln_lang_symbol_node_t *sym;
    mln_u8_t s[256];

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
    if ((tmp = val->data.s) == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument 1.");
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
    if (val->data.s == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument 2.");
        return NULL;
    }

    mln_rc4_init(s, val->data.s->data, val->data.s->len);
    mln_rc4_calc(s, tmp->data, tmp->len);

    if ((ret_var = mln_lang_var_create_ref_string(ctx, tmp, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

