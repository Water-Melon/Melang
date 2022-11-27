
/*
 * Copyright (C) Niklaus F.Schen.
 */

#include "mln_lang_prime.h"
#include "mln_prime_generator.h"
#include "mln_core.h"
#include "mln_conf.h"

#ifdef __DEBUG__
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x);
#endif

static int melon_init_flag = 0;

static int mln_lang_prime(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static mln_lang_var_t *mln_lang_prime_process(mln_lang_ctx_t *ctx);

static int mln_lang_prime_global_init(void)
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
        cattr.global_init = mln_lang_prime_global_init;
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
    if (mln_lang_prime(ctx, mln_lang_var_val_get(obj)->data.obj) < 0) {
        mln_lang_var_free(obj);
        return NULL;
    }
    return obj;
}

static int mln_lang_prime(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("prime");
    mln_string_t v = mln_string("base");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_prime_process, NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v, M_LANG_VAR_NORMAL, val, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_val_free(val);
        mln_lang_func_detail_free(func);
        return -1;
    }
    func->args_head = func->args_tail = var;
    func->nargs = 1;
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

static mln_lang_var_t *mln_lang_prime_process(mln_lang_ctx_t *ctx)
{
    mln_u32_t p;
    mln_lang_val_t *val;
    mln_lang_var_t *ret_var;
    mln_string_t var = mln_string("base");
    mln_lang_symbol_node_t *sym;
    if ((sym = mln_lang_symbol_node_search(ctx, &var, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);

    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT) {
        mln_lang_errmsg(ctx, "Invalid argument.");
        return NULL;
    }
    val = sym->data.var->val;

    p = mln_prime_calc(val->data.i);

    if ((ret_var = mln_lang_var_create_int(ctx, p, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

