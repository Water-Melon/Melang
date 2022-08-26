
/*
 * Copyright (C) Niklaus F.Schen.
 */
#include "mln_lang_sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "mln_lex.h"
#include "mln_cron.h"
#include "mln_log.h"

#ifdef __DEBUG__
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x);
#endif

static int mln_lang_sys_resource_register(mln_lang_ctx_t *ctx);
static void mln_lang_sys_resource_unregister(mln_lang_ctx_t *ctx);
static int mln_lang_sys_size_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_size_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_keys_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_keys_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_keys_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata);
static int mln_lang_sys_merge_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_merge_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_merge_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata);
static int mln_lang_sys_diff_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_diff_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_key_diff_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_key_diff_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_diff_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata);
static int mln_lang_sys_diff_check_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata);
static int mln_lang_sys_has_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_has_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_int_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_int_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_real_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_real_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_str_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_str_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_nil_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_nil_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_bool_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_bool_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_obj_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_obj_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_func_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_func_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_is_array_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_is_array_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_int_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_int_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_bool_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_bool_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_real_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_real_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_str_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_str_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_obj_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_obj_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_obj_add_key(mln_rbtree_node_t *node, void *rn_data, void *udata);
static int mln_lang_sys_array_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_array_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_array_add_nil(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key);
static int mln_lang_sys_array_add_int(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_s64_t i, mln_lang_var_t *key);
static int mln_lang_sys_array_add_bool(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_u8_t b, mln_lang_var_t *key);
static int mln_lang_sys_array_add_real(mln_lang_ctx_t *ctx, mln_lang_array_t *array, double f, mln_lang_var_t *key);
static int mln_lang_sys_array_add_string(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_string_t *s, mln_lang_var_t *key);
static int mln_lang_sys_array_add_func(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_func_detail_t *f, mln_lang_var_t *key);
static int mln_lang_sys_array_add_elem(mln_rbtree_node_t *node, void *rn_data, void *udata);
static int mln_lang_sys_type_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_type_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_getproperty_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_getproperty_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_setproperty_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_setproperty_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_eval_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_eval_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_remove_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_remove_process(mln_lang_ctx_t *ctx);
static inline int mln_sys_remove_is_dir(char * filename);
static int mln_sys_remove_delete_dir(char * dirname);
static int mln_sys_remove(char *pathname);
static int mln_lang_sys_mkdir_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_mkdir_process(mln_lang_ctx_t *ctx);
static inline void mln_lang_sys_mkdir_get_prio(mln_s64_t prio, mode_t *mode);
static int mln_lang_sys_exist_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_exist_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_lsdir_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_lsdir_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_isdir_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_isdir_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_time_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_time_process(mln_lang_ctx_t *ctx);
static int mln_lang_sys_cron_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_cron_process(mln_lang_ctx_t *ctx);
#if !defined(WIN32)
static mln_lang_sys_exec_t *mln_lang_sys_exec_new(mln_lang_ctx_t *ctx, mln_rbtree_t *tree, int fd, mln_s64_t size_limit);
static void mln_lang_sys_exec_free(mln_lang_sys_exec_t *se);
static int mln_lang_sys_exec_cmp(mln_lang_sys_exec_t *se1, mln_lang_sys_exec_t *se2);
static int mln_lang_sys_exec_handler(mln_lang_ctx_t *ctx);
static mln_lang_var_t *mln_lang_sys_exec_process(mln_lang_ctx_t *ctx);
static void mln_lang_sys_exec_read_handler(mln_event_t *ev, int fd, void *data);
#endif

int mln_lang_sys(mln_lang_ctx_t *ctx)
{
    if (mln_lang_sys_resource_register(ctx) < 0) return -1;
    if (mln_lang_sys_size_handler(ctx) < 0) goto err;
    if (mln_lang_sys_keys_handler(ctx) < 0) goto err;
    if (mln_lang_sys_merge_handler(ctx) < 0) goto err;
    if (mln_lang_sys_diff_handler(ctx) < 0) goto err;
    if (mln_lang_sys_key_diff_handler(ctx) < 0) goto err;
    if (mln_lang_sys_has_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_int_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_real_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_str_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_nil_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_bool_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_obj_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_func_handler(ctx) < 0) goto err;
    if (mln_lang_sys_is_array_handler(ctx) < 0) goto err;
    if (mln_lang_sys_int_handler(ctx) < 0) goto err;
    if (mln_lang_sys_bool_handler(ctx) < 0) goto err;
    if (mln_lang_sys_real_handler(ctx) < 0) goto err;
    if (mln_lang_sys_str_handler(ctx) < 0) goto err;
    if (mln_lang_sys_obj_handler(ctx) < 0) goto err;
    if (mln_lang_sys_array_handler(ctx) < 0) goto err;
    if (mln_lang_sys_type_handler(ctx) < 0) goto err;
    if (mln_lang_sys_getproperty_handler(ctx) < 0) goto err;
    if (mln_lang_sys_setproperty_handler(ctx) < 0) goto err;
    if (mln_lang_sys_eval_handler(ctx) < 0) goto err;
    if (mln_lang_sys_remove_handler(ctx) < 0) goto err;
    if (mln_lang_sys_mkdir_handler(ctx) < 0) goto err;
    if (mln_lang_sys_exist_handler(ctx) < 0) goto err;
    if (mln_lang_sys_lsdir_handler(ctx) < 0) goto err;
    if (mln_lang_sys_isdir_handler(ctx) < 0) goto err;
    if (mln_lang_sys_time_handler(ctx) < 0) goto err;
    if (mln_lang_sys_cron_handler(ctx) < 0) goto err;
#if !defined(WIN32)
    if (mln_lang_sys_exec_handler(ctx) < 0) goto err;
#endif
    return 0;

err:
    mln_lang_sys_resource_unregister(ctx);
    return -1;
}

static int mln_lang_sys_resource_register(mln_lang_ctx_t *ctx)
{
    mln_rbtree_t *tree;
    struct mln_rbtree_attr rbattr;

#if !defined(WIN32)
    rbattr.pool = ctx->pool;
    rbattr.pool_alloc = (rbtree_pool_alloc_handler)mln_alloc_m;
    rbattr.pool_free = (rbtree_pool_free_handler)mln_alloc_free;
    rbattr.cmp = (rbtree_cmp)mln_lang_sys_exec_cmp;
    rbattr.data_free = (rbtree_free_data)mln_lang_sys_exec_free;
    rbattr.cache = 0;
    if ((tree = mln_rbtree_init(&rbattr)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
    if (mln_lang_ctx_resource_register(ctx, "sys_exec", tree, (mln_lang_resource_free)mln_rbtree_destroy) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_rbtree_destroy(tree);
        return -1;
    }
#endif
    return 0;
}

static void mln_lang_sys_resource_unregister(mln_lang_ctx_t *ctx)
{
#if !defined(WIN32)
#endif
}

static int mln_lang_sys_array_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_array");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_array_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_array_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type == M_LANG_SYMBOL_VAR) {
        mln_s32_t type = mln_lang_var_val_type_get(sym->data.var);
        mln_lang_val_t *val = mln_lang_var_val_get(sym->data.var);
        if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        mln_lang_array_t *array = ret_var->val->data.array;
        switch (type) {
            case M_LANG_VAL_TYPE_NIL:
                if (mln_lang_sys_array_add_nil(ctx, array, NULL) < 0) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_INT:
                if (mln_lang_sys_array_add_int(ctx, array, val->data.i, NULL) < 0) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_BOOL:
                if (mln_lang_sys_array_add_bool(ctx, array, val->data.b, NULL) < 0) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_REAL:
                if (mln_lang_sys_array_add_real(ctx, array, val->data.f, NULL) < 0) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_STRING:
                if (mln_lang_sys_array_add_string(ctx, array, val->data.s, NULL) < 0) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_OBJECT:
                if (mln_rbtree_scan_all(val->data.obj->members, mln_lang_sys_array_add_elem, array) < 0) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_FUNC:
                if (mln_lang_sys_array_add_func(ctx, array, val->data.func, NULL) < 0) {
                    mln_lang_var_free(ret_var);
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_ARRAY:
            {
                mln_lang_var_free(ret_var);
                if ((ret_var = mln_lang_var_dup(ctx, sym->data.var)) == NULL) {
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            }
            default:
                ASSERT(0);
                mln_lang_var_free(ret_var);
                mln_lang_errmsg(ctx, "Cannot convert to ARRAY.");
                return NULL;
        }
        return ret_var;
    }
    mln_lang_errmsg(ctx, "Cannot convert to ARRAY.");
    return NULL;
}

static int mln_lang_sys_array_add_nil(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key)
{
    return mln_lang_array_get(ctx, array, key) == NULL ? -1 : 0;
}

static int mln_lang_sys_array_add_int(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_s64_t i, mln_lang_var_t *key)
{
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    if ((array_val = mln_lang_array_get(ctx, array, key)) == NULL) {
         return -1;
    }
    var.type = M_LANG_VAR_NORMAL;
    var.name = NULL;
    var.val = &val;
    var.in_set = NULL;
    var.prev = var.next = NULL;
    val.data.i = i;
    val.type = M_LANG_VAL_TYPE_INT;
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_array_add_bool(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_u8_t b, mln_lang_var_t *key)
{
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    if ((array_val = mln_lang_array_get(ctx, array, key)) == NULL) {
        return -1;
    }
    var.type = M_LANG_VAR_NORMAL;
    var.name = NULL;
    var.val = &val;
    var.in_set = NULL;
    var.prev = var.next = NULL;
    val.data.b = b;
    val.type = M_LANG_VAL_TYPE_BOOL;
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_array_add_real(mln_lang_ctx_t *ctx, mln_lang_array_t *array, double f, mln_lang_var_t *key)
{
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    if ((array_val = mln_lang_array_get(ctx, array, key)) == NULL) {
         return -1;
    }
    var.type = M_LANG_VAR_NORMAL;
    var.name = NULL;
    var.val = &val;
    var.in_set = NULL;
    var.prev = var.next = NULL;
    val.data.f = f;
    val.type = M_LANG_VAL_TYPE_REAL;
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_array_add_string(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_string_t *s, mln_lang_var_t *key)
{
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    if ((array_val = mln_lang_array_get(ctx, array, key)) == NULL) {
        return -1;
    }
    var.type = M_LANG_VAR_NORMAL;
    var.name = NULL;
    var.val = &val;
    var.in_set = NULL;
    var.prev = var.next = NULL;
    val.data.s = s;
    val.type = M_LANG_VAL_TYPE_STRING;
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_array_add_func(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_func_detail_t *f, mln_lang_var_t *key)
{
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    if ((array_val = mln_lang_array_get(ctx, array, key)) == NULL) {
        return -1;
    }
    var.type = M_LANG_VAR_NORMAL;
    var.name = NULL;
    var.val = &val;
    var.in_set = NULL;
    var.prev = var.next = NULL;
    val.data.func = f;
    val.type = M_LANG_VAL_TYPE_FUNC;
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_array_add_elem(mln_rbtree_node_t *node, void *rn_data, void *udata)
{
    mln_lang_array_t *array = (mln_lang_array_t *)udata;
    mln_lang_ctx_t *ctx = array->ctx;
    mln_lang_var_t *prop = (mln_lang_var_t *)rn_data;
    ASSERT(prop->name != NULL);
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    var.type = M_LANG_VAR_NORMAL;
    var.name = NULL;
    var.val = &val;
    var.in_set = NULL;
    var.prev = var.next = NULL;
    val.data.s = prop->name;
    val.type = M_LANG_VAL_TYPE_STRING;
    val.ref = 1;
    if ((array_val = mln_lang_array_get(ctx, array, &var)) == NULL) {
        return -1;
    }
    if (mln_lang_var_value_set(ctx, array_val, prop) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_obj_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_obj");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_obj_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_obj_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type == M_LANG_SYMBOL_VAR) {
        mln_s32_t type = mln_lang_var_val_type_get(sym->data.var);
        if (type == M_LANG_VAL_TYPE_OBJECT) {
            if ((ret_var = mln_lang_var_dup(ctx, sym->data.var)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        } else if (type == M_LANG_VAL_TYPE_ARRAY) {
            mln_lang_val_t *val = mln_lang_var_val_get(sym->data.var);
            mln_lang_array_t *array = val->data.array;
            if ((ret_var = mln_lang_var_create_obj(ctx, NULL, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            if (mln_rbtree_scan_all(array->elems_key, \
                                    mln_lang_sys_obj_add_key, \
                                    ret_var->val->data.obj) < 0) {
                mln_lang_var_free(ret_var);
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        } else {
            mln_lang_errmsg(ctx, "Cannot convert to OBJECT.");
            return NULL;
        }
    }
    mln_lang_errmsg(ctx, "Cannot convert to OBJECT.");
    return NULL;
}

static int mln_lang_sys_obj_add_key(mln_rbtree_node_t *node, void *rn_data, void *udata)
{
    mln_lang_var_t tmp;
    mln_lang_object_t *obj = (mln_lang_object_t *)udata;
    mln_lang_ctx_t *ctx = obj->ctx;
    mln_lang_array_elem_t *ae = (mln_lang_array_elem_t *)rn_data;
    if (mln_lang_var_val_type_get(ae->key) != M_LANG_VAL_TYPE_STRING) {
        return 0;
    }
    tmp.type = M_LANG_VAR_NORMAL;
    tmp.name = ae->key->val->data.s;
    tmp.val = ae->value->val;
    tmp.in_set = NULL;
    tmp.prev = NULL;
    tmp.next = NULL;
    return mln_lang_object_add_member(ctx, obj, &tmp);
}

static int mln_lang_sys_str_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_str");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_str_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_str_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type == M_LANG_SYMBOL_VAR) {
        mln_s32_t type = mln_lang_var_val_type_get(sym->data.var);
        mln_lang_val_t *val = mln_lang_var_val_get(sym->data.var);
        mln_size_t n = 0;
        char buf[1024] = {0};
        mln_string_t s;
        switch (type) {
            case M_LANG_VAL_TYPE_INT:
#if defined(WIN32)
                n = snprintf(buf, sizeof(buf)-1, "%I64d", val->data.i);
#elif defined(i386) || defined(__arm__)
                n = snprintf(buf, sizeof(buf)-1, "%lld", val->data.i);
#else
                n = snprintf(buf, sizeof(buf)-1, "%ld", val->data.i);
#endif
                mln_string_nset(&s, buf, n);
                if ((ret_var = mln_lang_var_create_string(ctx, &s, NULL)) == NULL) {
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_BOOL:
                if (mln_lang_condition_is_true(sym->data.var)) {
                    n = snprintf(buf, sizeof(buf)-1, "true");
                    mln_string_nset(&s, buf, n);
                    if ((ret_var = mln_lang_var_create_string(ctx, &s, NULL)) == NULL) {
                        mln_lang_errmsg(ctx, "No memory.");
                        return NULL;
                    }
                } else {
                    n = snprintf(buf, sizeof(buf)-1, "false");
                    mln_string_nset(&s, buf, n);
                    if ((ret_var = mln_lang_var_create_string(ctx, &s, NULL)) == NULL) {
                        mln_lang_errmsg(ctx, "No memory.");
                        return NULL;
                    }
                }
                break;
            case M_LANG_VAL_TYPE_REAL:
                n = snprintf(buf, sizeof(buf)-1, "%lf", val->data.f);
                mln_string_nset(&s, buf, n);
                if ((ret_var = mln_lang_var_create_string(ctx, &s, NULL)) == NULL) {
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            case M_LANG_VAL_TYPE_STRING:
                if ((ret_var = mln_lang_var_create_ref_string(ctx, val->data.s, NULL)) == NULL) {
                    mln_lang_errmsg(ctx, "No memory.");
                    return NULL;
                }
                break;
            default:
                mln_lang_errmsg(ctx, "Cannot convert to STRING.");
                return NULL;
        }
        return ret_var;
    }
    mln_lang_errmsg(ctx, "Cannot convert to STRING.");
    return NULL;
}

static int mln_lang_sys_real_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_real");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_real_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_real_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type == M_LANG_SYMBOL_VAR) {
        mln_s32_t type;
        mln_lang_val_t *val = mln_lang_var_val_get(sym->data.var);
        if ((type = mln_lang_var_val_type_get(sym->data.var)) == M_LANG_VAL_TYPE_INT) {
            if ((ret_var = mln_lang_var_create_real(ctx, val->data.i, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        } else if (type == M_LANG_VAL_TYPE_REAL) {
            if ((ret_var = mln_lang_var_create_real(ctx, val->data.f, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        } else if (type == M_LANG_VAL_TYPE_STRING) {
            double f;
            mln_s8ptr_t buf;
            if ((buf = (mln_s8ptr_t)malloc(val->data.s->len+1)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            memcpy(buf, val->data.s->data, val->data.s->len);
            buf[val->data.s->len] = 0;
            f = atof(buf);
            free(buf);
            if ((ret_var = mln_lang_var_create_real(ctx, f, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        }
    }
    mln_lang_errmsg(ctx, "Cannot convert to REAL.");
    return NULL;
}

static int mln_lang_sys_bool_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_bool");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_bool_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_bool_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type == M_LANG_SYMBOL_VAR) {
        if (mln_lang_condition_is_true(sym->data.var)) {
            ret_var = mln_lang_var_create_true(ctx, NULL);
        } else {
            ret_var = mln_lang_var_create_false(ctx, NULL);
        }
        if (ret_var == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        return ret_var;
    }
    mln_lang_errmsg(ctx, "Cannot convert to BOOLEAN.");
    return NULL;
}

static int mln_lang_sys_int_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_int");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_int_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_int_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type == M_LANG_SYMBOL_VAR) {
        mln_s32_t type;
        mln_lang_val_t *val = mln_lang_var_val_get(sym->data.var);
        if ((type = mln_lang_var_val_type_get(sym->data.var)) == M_LANG_VAL_TYPE_INT) {
            if ((ret_var = mln_lang_var_create_int(ctx, val->data.i, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        } else if (type == M_LANG_VAL_TYPE_REAL) {
            if ((ret_var = mln_lang_var_create_int(ctx, val->data.f, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        } else if (type == M_LANG_VAL_TYPE_STRING) {
            mln_s64_t i;
            mln_s8ptr_t buf;
            if ((buf = (mln_s8ptr_t)malloc(val->data.s->len+1)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            memcpy(buf, val->data.s->data, val->data.s->len);
            buf[val->data.s->len] = 0;
#if defined(i386) || defined(__arm__) || defined(WIN32)
            i = atoll(buf);
#else
            i = atol(buf);
#endif
            free(buf);
            if ((ret_var = mln_lang_var_create_int(ctx, i, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
            return ret_var;
        }
    }
    mln_lang_errmsg(ctx, "Cannot convert to INT.");
    return NULL;
}

static int mln_lang_sys_is_int_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_int");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_int_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_int_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_is_real_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_real");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_real_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_real_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_REAL)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_is_str_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_str");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_str_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_str_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_is_nil_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_nil");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_nil_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_nil_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_NIL)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_is_bool_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_bool");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_bool_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_bool_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_BOOL)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_is_obj_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_obj");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_obj_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_obj_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_OBJECT)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_is_func_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_func");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_func_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_func_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_FUNC)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_is_array_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_is_array");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_is_array_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_is_array_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY)
    {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_size_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_size");
    mln_string_t v1 = mln_string("array");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_size_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_size_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("array");
    mln_lang_symbol_node_t *sym;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || \
        mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY)
    {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    if ((ret_var = mln_lang_var_create_int(ctx, \
                                             sym->data.var->val->data.array->elems_index->nr_node, \
                                             NULL)) == NULL)
    {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_has_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_has");
    mln_string_t v1 = mln_string("owner");
    mln_string_t v2 = mln_string("thing");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_has_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_has_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("owner");
    mln_string_t v2 = mln_string("thing");
    mln_lang_symbol_node_t *sym, *sym2;
    mln_s32_t type;

    if ((sym2 = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        return NULL;
    }
    if (sym2->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    type = mln_lang_var_val_type_get(sym->data.var);
    if (type == M_LANG_VAL_TYPE_ARRAY) {
        if (mln_lang_array_elem_exist(mln_lang_var_val_get(sym->data.var)->data.array, sym2->data.var)) {
            ret_var = mln_lang_var_create_true(ctx, NULL);
        } else {
            ret_var = mln_lang_var_create_false(ctx, NULL);
        }
    } else if (type == M_LANG_VAL_TYPE_OBJECT) {
        if (mln_lang_var_val_type_get(sym2->data.var) != M_LANG_VAL_TYPE_STRING) {
            mln_lang_errmsg(ctx, "Invalid type of argument 2.");
            return NULL;
        }
        if (mln_lang_set_member_search(mln_lang_var_val_get(sym->data.var)->data.obj->members, \
                                       mln_lang_var_val_get(sym2->data.var)->data.s) == NULL)
        {
            ret_var = mln_lang_var_create_false(ctx, NULL);
        } else {
            ret_var = mln_lang_var_create_true(ctx, NULL);
        }
    } else {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_keys_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_keys");
    mln_string_t v1 = mln_string("array");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_keys_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_keys_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("array");
    mln_lang_symbol_node_t *sym;
    mln_lang_array_t *array, *arr;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    array = mln_lang_var_val_get(sym->data.var)->data.array;
    if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    arr = ret_var->val->data.array;
    if (mln_rbtree_scan_all(array->elems_key, mln_lang_sys_keys_scanner, arr) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_keys_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata)
{
    mln_lang_array_t *array = (mln_lang_array_t *)udata;
    mln_lang_array_elem_t *elem = (mln_lang_array_elem_t *)rn_data;
    mln_lang_var_t *var;

    if ((var = mln_lang_array_get(array->ctx, array, NULL)) == NULL) {
        return -1;
    }
    if (mln_lang_var_value_set(array->ctx, var, elem->key) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_merge_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_merge");
    mln_string_t v1 = mln_string("array1");
    mln_string_t v2 = mln_string("array2");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_merge_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_merge_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("array1");
    mln_string_t v2 = mln_string("array2");
    mln_lang_symbol_node_t *sym;
    mln_lang_array_t *array, *arr;

    if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    arr = ret_var->val->data.array;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    array = mln_lang_var_val_get(sym->data.var)->data.array;
    if (mln_rbtree_scan_all(array->elems_index, mln_lang_sys_merge_scanner, arr) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(ret_var);
        return NULL;
    }

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    array = mln_lang_var_val_get(sym->data.var)->data.array;
    if (mln_rbtree_scan_all(array->elems_index, mln_lang_sys_merge_scanner, arr) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_merge_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata)
{
    mln_lang_array_t *array = (mln_lang_array_t *)udata;
    mln_lang_array_elem_t *elem = (mln_lang_array_elem_t *)rn_data;
    mln_lang_var_t *var;

    if ((var = mln_lang_array_get(array->ctx, array, elem->key)) == NULL) {
        return -1;
    }
    if (mln_lang_var_value_set(array->ctx, var, elem->value) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_diff_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_diff");
    mln_string_t v1 = mln_string("arr1");
    mln_string_t v2 = mln_string("arr2");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_diff_process, NULL, NULL)) == NULL) {
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
    mln_lang_var_chain_add(&(func->args_head), &(func->args_tail), var);
    ++func->nargs;
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v2, M_LANG_VAR_REFER, val, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_diff_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("arr1");
    mln_string_t v2 = mln_string("arr2");
    mln_lang_symbol_node_t *sym;
    struct mln_sys_diff_s udata;
    mln_lang_array_t *arr;

    if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    udata.dest = ret_var->val->data.array;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    arr = mln_lang_var_val_get(sym->data.var)->data.array;

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    udata.notin = mln_lang_var_val_get(sym->data.var)->data.array;
    udata.key = 0;

    if (mln_rbtree_scan_all(arr->elems_index, mln_lang_sys_diff_scanner, &udata) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_key_diff_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_key_diff");
    mln_string_t v1 = mln_string("arr1");
    mln_string_t v2 = mln_string("arr2");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_key_diff_process, NULL, NULL)) == NULL) {
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
    mln_lang_var_chain_add(&(func->args_head), &(func->args_tail), var);
    ++func->nargs;
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v2, M_LANG_VAR_REFER, val, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_key_diff_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("arr1");
    mln_string_t v2 = mln_string("arr2");
    mln_lang_symbol_node_t *sym;
    struct mln_sys_diff_s udata;
    mln_lang_array_t *arr;

    if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    udata.dest = ret_var->val->data.array;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    arr = mln_lang_var_val_get(sym->data.var)->data.array;

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    udata.notin = mln_lang_var_val_get(sym->data.var)->data.array;
    udata.key = 1;

    if (mln_rbtree_scan_all(arr->elems_key, mln_lang_sys_diff_scanner, &udata) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_diff_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata)
{
    struct mln_sys_diff_s *sd = (struct mln_sys_diff_s *)udata;
    mln_lang_array_elem_t *elem = (mln_lang_array_elem_t *)rn_data;
    mln_lang_var_t *var;

    if (sd->key) {
        if (mln_lang_array_elem_exist(sd->notin, elem->key))
            return 0;
    } else {
        if (mln_rbtree_scan_all(sd->notin->elems_index, mln_lang_sys_diff_check_scanner, elem) < 0)
            return 0;
    }

    if (elem->key != NULL) {
        var = mln_lang_array_get(sd->dest->ctx, sd->dest, elem->key);
    } else {
        var = mln_lang_array_get(sd->dest->ctx, sd->dest, NULL);
    }
    if (var == NULL) return -1;
    if (mln_lang_var_value_set(sd->dest->ctx, var, elem->value) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_sys_diff_check_scanner(mln_rbtree_node_t *node, void *rn_data, void *udata)
{
    mln_lang_array_elem_t *checked = (mln_lang_array_elem_t *)udata;
    mln_lang_array_elem_t *elem = (mln_lang_array_elem_t *)rn_data;
    if (checked->value == elem->value || checked->value->val == elem->value->val) return -1;
    return mln_lang_var_cmp(elem->value, checked->value) == 0? -1: 0;
}

static mln_lang_var_t *mln_lang_sys_type_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("var");
    mln_lang_symbol_node_t *sym;
    mln_s32_t type;
    mln_string_t tmp;
    mln_lang_val_t *val;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type == M_LANG_SYMBOL_SET) {
        mln_string_set(&tmp, "set");
    } else {
        type = mln_lang_var_val_type_get(sym->data.var);
        val = sym->data.var->val;
        switch (type) {
            case M_LANG_VAL_TYPE_NIL:
                mln_string_set(&tmp, "nil");
                break;
            case M_LANG_VAL_TYPE_INT:
                mln_string_set(&tmp, "int");
                break;
            case M_LANG_VAL_TYPE_BOOL:
                mln_string_set(&tmp, "bool");
                break;
            case M_LANG_VAL_TYPE_REAL:
                mln_string_set(&tmp, "real");
                break;
            case M_LANG_VAL_TYPE_STRING:
                mln_string_set(&tmp, "string");
                break;
            case M_LANG_VAL_TYPE_OBJECT:
                if (val->data.obj->in_set == NULL || val->data.obj->in_set->name == NULL) {
                    mln_string_set(&tmp, "object");
                } else {
                    mln_string_nset(&tmp, val->data.obj->in_set->name->data, val->data.obj->in_set->name->len);
                }
                break;
            case M_LANG_VAL_TYPE_FUNC:
                mln_string_set(&tmp, "function");
                break;
            case M_LANG_VAL_TYPE_ARRAY:
                mln_string_set(&tmp, "array");
                break;
            default:
                mln_string_set(&tmp, "unknown");
                break;
        }
    }
    if ((ret_var = mln_lang_var_create_string(ctx, &tmp, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_type_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_type");
    mln_string_t v1 = mln_string("var");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_type_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static int mln_lang_sys_getproperty_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_get_property");
    mln_string_t v1 = mln_string("obj");
    mln_string_t v2 = mln_string("prop");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_getproperty_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_getproperty_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v1 = mln_string("obj");
    mln_string_t v2 = mln_string("prop");
    mln_lang_symbol_node_t *sym, *sym2;
    mln_lang_var_t *var;

    if ((sym2 = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        return NULL;
    }
    if (sym2->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_OBJECT) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    if (mln_lang_var_val_type_get(sym2->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    var = mln_lang_set_member_search(mln_lang_var_val_get(sym->data.var)->data.obj->members, \
                                     mln_lang_var_val_get(sym2->data.var)->data.s);
    if (var == NULL) {
        mln_lang_errmsg(ctx, "No such property in object.");
        return NULL;
    }
    return mln_lang_var_ref(var);
}

static int mln_lang_sys_setproperty_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_set_property");
    mln_string_t v1 = mln_string("obj");
    mln_string_t v2 = mln_string("prop");
    mln_string_t v3 = mln_string("val");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_setproperty_process, NULL, NULL)) == NULL) {
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
    if ((val = mln_lang_val_new(ctx, M_LANG_VAL_TYPE_NIL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_func_detail_free(func);
        return -1;
    }
    if ((var = mln_lang_var_new(ctx, &v3, M_LANG_VAR_REFER, val, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_setproperty_process(mln_lang_ctx_t *ctx)
{
    mln_string_t v1 = mln_string("obj");
    mln_string_t v2 = mln_string("prop");
    mln_string_t v3 = mln_string("val");
    mln_lang_symbol_node_t *sym;
    mln_lang_var_t *var;
    mln_lang_val_t *val1, *val2;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_OBJECT) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val1 = mln_lang_var_val_get(sym->data.var);

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    val2 = mln_lang_var_val_get(sym->data.var);

    if ((sym = mln_lang_symbol_node_search(ctx, &v3, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 3 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 3.");
        return NULL;
    }

    var = mln_lang_set_member_search(val1->data.obj->members, val2->data.s);
    if (var == NULL) {
        mln_lang_var_t tmpvar;
        mln_lang_val_t tmpval;
        mln_rbtree_node_t *rn;
        tmpvar.type = M_LANG_VAR_NORMAL;
        tmpvar.name = val2->data.s;
        tmpvar.val = &tmpval;
        tmpvar.in_set = val1->data.obj->in_set;
        tmpvar.prev = tmpvar.next = NULL;
        tmpval.data.s = NULL;
        tmpval.type = M_LANG_VAL_TYPE_NIL;
        tmpval.ref = 1;
        if ((var = mln_lang_var_dup(ctx, &tmpvar)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        if ((rn = mln_rbtree_node_new(val1->data.obj->members, var)) == NULL) {
            mln_lang_var_free(var);
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        mln_rbtree_insert(val1->data.obj->members, rn);
    }
    if (mln_lang_var_value_set(ctx, var, sym->data.var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return mln_lang_var_ref(var);
}

static int mln_lang_sys_eval_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_eval");
    mln_string_t v1 = mln_string("val");
    mln_string_t v2 = mln_string("data");
    mln_string_t v3 = mln_string("in_string");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_eval_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_eval_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("val");
    mln_string_t v2 = mln_string("data");
    mln_string_t v3 = mln_string("in_string");
    mln_string_t data_name = mln_string("EVAL_DATA"), *dup;
    mln_lang_symbol_node_t *sym;
    mln_lang_val_t *val1, *val2;
    mln_s32_t type, type3;
    mln_u32_t job_type;
    mln_u8ptr_t data;
    mln_lang_ctx_t *newctx;
    /*arg1*/
    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val1 = mln_lang_var_val_get(sym->data.var);
    /*arg2*/
    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    val2 = mln_lang_var_val_get(sym->data.var);
    type = mln_lang_var_val_type_get(sym->data.var);
    if (type < M_LANG_VAL_TYPE_NIL || type >= M_LANG_VAL_TYPE_OBJECT) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2, only support nil, int, real, boolean, string.");
        return NULL;
    }
    /*arg3*/
    if ((sym = mln_lang_symbol_node_search(ctx, &v3, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 3 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 3.");
        return NULL;
    }
    type3 = mln_lang_var_val_type_get(sym->data.var);
    if (type3 == M_LANG_VAL_TYPE_BOOL && mln_lang_var_val_get(sym->data.var)->data.b) {
        job_type = M_INPUT_T_BUF;
    } else {
        job_type = M_INPUT_T_FILE;
    }
    /*create job ctx*/
    if ((newctx = mln_lang_job_new(ctx->lang, job_type, val1->data.s, NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "Eval failed..");
        return NULL;
    }
    switch (type) {
        case M_LANG_VAL_TYPE_NIL:
            data = NULL;
            break;
        case M_LANG_VAL_TYPE_INT:
            data = (mln_u8ptr_t)&(val2->data.i);
            break;
        case M_LANG_VAL_TYPE_BOOL:
            data = (mln_u8ptr_t)&(val2->data.b);
            break;
        case M_LANG_VAL_TYPE_REAL:
            data = (mln_u8ptr_t)&(val2->data.f);
            break;
        case M_LANG_VAL_TYPE_STRING:
            if ((data = (mln_u8ptr_t)mln_string_pool_dup(newctx->pool, val2->data.s)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                mln_lang_job_free(newctx);
                return NULL;
            }
            break;
        default:
            mln_lang_errmsg(ctx, "Invalid type of argument 2");
            ASSERT(0);
            return NULL;
    }
    dup = mln_string_pool_dup(newctx->pool, &data_name);
    if (dup == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        if (type == M_LANG_VAL_TYPE_STRING) mln_string_free((mln_string_t *)data);
        mln_lang_job_free(newctx);
        return NULL;
    }
    if (mln_lang_ctx_global_var_add(newctx, dup, data, type) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_string_free(dup);
        if (type == M_LANG_VAL_TYPE_STRING) mln_string_free((mln_string_t *)data);
        mln_lang_job_free(newctx);
        return NULL;
    }
    mln_string_free(dup);
    mln_lang_run(newctx->lang);
    if ((ret_var = mln_lang_var_create_nil(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_job_free(newctx);
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_remove_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_remove");
    mln_string_t v1 = mln_string("path");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_remove_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_remove_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("path");
    mln_lang_symbol_node_t *sym;
    mln_lang_val_t *val;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val = mln_lang_var_val_get(sym->data.var);
    if (mln_sys_remove((char *)val->data.s->data)) {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static inline int mln_sys_remove_is_dir(char * filename)
{
    struct stat buf;
    int ret = stat(filename,&buf);
    if (ret) return -1;
    if(buf.st_mode & S_IFDIR) return 0;
    return 1;
}

static int mln_sys_remove_delete_dir(char * dirname)
{
    char buf[1024];
    DIR * dir = NULL;
    struct dirent *ptr;
    int ret = 0;

    if ((dir = opendir(dirname)) == NULL) {
        return -1;
    }
    while((ptr = readdir(dir)) != NULL) {
        if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, "..")) continue;
        ret = snprintf(buf, sizeof(buf)-1, "%s/%s", dirname, ptr->d_name);
        buf[ret] = 0;
        if (!(ret = mln_sys_remove_is_dir(buf))) {
            if (mln_sys_remove_delete_dir(buf) < 0) return -1;
        } else if (ret == 1) {
            if (unlink(buf)) return -1;
        }
    }
    (void)closedir(dir);
    return rmdir(dirname);
}

static int mln_sys_remove(char *pathname)
{
    int ret = mln_sys_remove_is_dir(pathname);
    if (!ret) return mln_sys_remove_delete_dir(pathname);
    if (ret < 0) return -1;
    return unlink(pathname);
}

static int mln_lang_sys_mkdir_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_mkdir");
    mln_string_t v1 = mln_string("path");
    mln_string_t v2 = mln_string("mode");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_mkdir_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_mkdir_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("path"), v2 = mln_string("mode");
    mln_lang_symbol_node_t *sym;
    mln_lang_val_t *val1, *val2;
    mln_s32_t type;
    mode_t mode;
    mln_s64_t prio;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val1 = mln_lang_var_val_get(sym->data.var);

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    val2 = mln_lang_var_val_get(sym->data.var);
    type = mln_lang_var_val_type_get(sym->data.var);
    if (type == M_LANG_VAL_TYPE_INT) {
        prio = val2->data.i;
    } else if (type == M_LANG_VAL_TYPE_NIL) {
        prio = 0755;
    } else {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }

    mln_lang_sys_mkdir_get_prio(prio, &mode);

#if defined(WIN32)
    if (mkdir((char *)val1->data.s->data)) {
#else
    if (mkdir((char *)val1->data.s->data, mode)) {
#endif
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static inline void mln_lang_sys_mkdir_get_prio(mln_s64_t prio, mode_t *mode)
{
    mode_t m = 0;
    if (prio & 0x1) m |= S_IXOTH;
    if (prio & 0x2) m |= S_IWOTH;
    if (prio & 0x4) m |= S_IROTH;
    if (prio & 0x8) m |= S_IXGRP;
    if (prio & 0x10) m |= S_IWGRP;
    if (prio & 0x20) m |= S_IRGRP;
    if (prio & 0x40) m |= S_IXUSR;
    if (prio & 0x80) m |= S_IWUSR;
    if (prio & 0x100) m |= S_IRUSR;
    *mode = m;
}

static int mln_lang_sys_exist_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_exist");
    mln_string_t v1 = mln_string("path");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_exist_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_exist_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("path");
    mln_lang_symbol_node_t *sym;
    mln_lang_val_t *val;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val = mln_lang_var_val_get(sym->data.var);
    if (access((char *)val->data.s->data, F_OK)) {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_lsdir_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_lsdir");
    mln_string_t v1 = mln_string("path");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_lsdir_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_lsdir_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("path");
    mln_lang_symbol_node_t *sym;
    mln_lang_val_t *val;
    struct stat st;
    DIR *dp;
    struct dirent *de;
    mln_lang_array_t *arr;
    mln_string_t s;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val = mln_lang_var_val_get(sym->data.var);

    stat((char *)val->data.s->data, &st);
    if (!S_ISDIR(st.st_mode) || (dp = opendir((char *)val->data.s->data)) == NULL) {
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
    } else {
        if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
            closedir(dp);
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        arr = ret_var->val->data.array;
        while ((de = readdir(dp)) != NULL) {
            mln_string_set(&s, de->d_name);
            if (mln_lang_sys_array_add_string(ctx, arr, &s, NULL) < 0) {
                mln_lang_var_free(ret_var);
                closedir(dp);
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
        }
        closedir(dp);
    }
    return ret_var;
}

static int mln_lang_sys_isdir_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_isdir");
    mln_string_t v1 = mln_string("path");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_isdir_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_isdir_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("path");
    mln_lang_symbol_node_t *sym;
    mln_lang_val_t *val;
    struct stat st;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val = mln_lang_var_val_get(sym->data.var);

    stat((char *)val->data.s->data, &st);
    if (S_ISDIR(st.st_mode)) {
        ret_var = mln_lang_var_create_true(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_time_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_time");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_time_process, NULL, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return -1;
    }
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_time_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;

    if ((ret_var = mln_lang_var_create_int(ctx, (mln_s64_t)time(NULL), NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

static int mln_lang_sys_cron_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_cron");
    mln_string_t v1 = mln_string("exp");
    mln_string_t v2 = mln_string("timestamp");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_cron_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_cron_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("exp"), v2 = mln_string("timestamp");
    mln_lang_symbol_node_t *sym;
    mln_lang_val_t *val;
    time_t next;
    mln_string_t *dup;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    val = mln_lang_var_val_get(sym->data.var);

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_INT) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }

    if ((dup = mln_string_pool_dup(ctx->pool, val->data.s)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    next = mln_cron_parse(dup, (time_t)(mln_lang_var_val_get(sym->data.var)->data.i));
    mln_string_free(dup);
    if (!next) {
        ret_var = mln_lang_var_create_false(ctx, NULL);
    } else {
        ret_var = mln_lang_var_create_int(ctx, (mln_s64_t)next, NULL);
    }
    if (ret_var == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    return ret_var;
}

#if !defined(WIN32)
static mln_lang_sys_exec_t *mln_lang_sys_exec_new(mln_lang_ctx_t *ctx, mln_rbtree_t *tree, int fd, mln_s64_t size_limit)
{
    mln_lang_sys_exec_t *se;
    if ((se = (mln_lang_sys_exec_t *)mln_alloc_m(ctx->pool, sizeof(mln_lang_sys_exec_t))) == NULL) {
        return NULL;
    }
    se->ctx = ctx;
    if (mln_tcp_conn_init(&se->conn, fd) < 0) {
        mln_alloc_free(se);
        return NULL;
    }
    se->size_limit = size_limit;
    se->cur_size = 0;
    se->head = se->tail = NULL;
    se->tree = tree;
    se->rn = NULL;
    return se;
}

static void mln_lang_sys_exec_free(mln_lang_sys_exec_t *se)
{
    if (se == NULL) return;
    int fd = mln_tcp_conn_get_fd(&se->conn);
    mln_event_set_fd(se->ctx->lang->ev, fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
    mln_chain_pool_release_all(se->head);
    mln_tcp_conn_destroy(&se->conn);
    close(fd);
    mln_alloc_free(se);
}

static int mln_lang_sys_exec_cmp(mln_lang_sys_exec_t *se1, mln_lang_sys_exec_t *se2)
{
    return mln_tcp_conn_get_fd(&se1->conn) - mln_tcp_conn_get_fd(&se2->conn);
}

static int mln_lang_sys_exec_handler(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("mln_exec");
    mln_string_t v1 = mln_string("cmd");
    mln_string_t v2 = mln_string("bufsize");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_sys_exec_process, NULL, NULL)) == NULL) {
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
    if (mln_lang_symbol_node_join(ctx, M_LANG_SYMBOL_VAR, var) < 0) {
        mln_lang_errmsg(ctx, "No memory.");
        mln_lang_var_free(var);
        return -1;
    }
    return 0;
}

static mln_lang_var_t *mln_lang_sys_exec_process(mln_lang_ctx_t *ctx)
{
    mln_lang_var_t *ret_var = NULL;
    mln_string_t v1 = mln_string("cmd"), v2 = mln_string("bufsize");
    mln_lang_symbol_node_t *sym;
    mln_string_t *cmd;
    mln_s64_t bufsize;
    mln_lang_sys_exec_t *se;
    mln_rbtree_node_t *rn;
    mln_rbtree_t *tree;
    pid_t pid;
    int fds[2];

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 1 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR || mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument 1.");
        return NULL;
    }
    cmd = mln_lang_var_val_get(sym->data.var)->data.s;

    if ((sym = mln_lang_symbol_node_search(ctx, &v2, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument 2 missing.");
        return NULL;
    }
    if (sym->type != M_LANG_SYMBOL_VAR) {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }
    if (mln_lang_var_val_type_get(sym->data.var) == M_LANG_VAL_TYPE_INT) {
        bufsize = mln_lang_var_val_get(sym->data.var)->data.i;
    } else if (mln_lang_var_val_type_get(sym->data.var) == M_LANG_VAL_TYPE_NIL) {
        bufsize = -1;
    } else {
        mln_lang_errmsg(ctx, "Invalid type of argument 2.");
        return NULL;
    }

    if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }

    if ((tree = mln_lang_ctx_resource_fetch(ctx, "sys_exec")) == NULL) {
        return ret_var;
    }

    signal(SIGCHLD, SIG_IGN);
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        mln_lang_errmsg(ctx, "create socket pair failed.");
        mln_lang_var_free(ret_var);
        return NULL;
    }
    if ((pid = fork()) > 0) {
        close(fds[1]);
        if ((se = mln_lang_sys_exec_new(ctx, tree, fds[0], bufsize)) == NULL) {
            mln_lang_errmsg(ctx, "create socket pair failed.");
            close(fds[0]);
            mln_lang_var_free(ret_var);
            return NULL;
        }
        if (mln_event_set_fd(ctx->lang->ev, fds[0], M_EV_RECV|M_EV_NONBLOCK, M_EV_UNLIMITED, se, mln_lang_sys_exec_read_handler) < 0) {
            mln_lang_errmsg(ctx, "No memory.");
            mln_lang_sys_exec_free(se);
            mln_lang_var_free(ret_var);
            return NULL;
        }
        if ((rn = mln_rbtree_node_new(tree, se)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            mln_lang_sys_exec_free(se);
            mln_lang_var_free(ret_var);
            return NULL;
        }
        mln_rbtree_insert(tree, rn);
        se->rn = rn;
    } else if (pid == 0) {
        /*
         * fds[0] must be closed. If not close, after command execution and the parent process
         * close and EPOLL_CTL_DEL fds[0], epoll will trigger fds[0] again,
         * which means a freed memory structure will be visited again.
         * so segment fault happened.
         */
        close(fds[0]);
        close(1);
        close(2);
        int rc = dup(fds[1]);
        rc = dup(fds[1]);
        if (rc < 0) rc = 0;
        close(fds[1]);
        signal(SIGCHLD, SIG_DFL);
        if (execl("/bin/sh", "sh", "-c", (char *)cmd->data, (char *)0) < 0) {
            exit(127);
        }
    } else {
        mln_lang_errmsg(ctx, "fork failed.");
        close(fds[0]);
        close(fds[1]);
        mln_lang_var_free(ret_var);
        return NULL;
    }

    mln_lang_ctx_suspend(ctx);
    return ret_var;
}

static void mln_lang_sys_exec_read_handler(mln_event_t *ev, int fd, void *data)
{
    mln_chain_t *c, *start, *last = NULL;
    mln_lang_sys_exec_t *se = (mln_lang_sys_exec_t *)data;
    mln_lang_ctx_t *ctx = se->ctx;
    mln_lang_var_t *ret_var;
    mln_u8ptr_t p;
    mln_string_t s;

    int rc = mln_tcp_conn_recv(&se->conn, M_C_TYPE_MEMORY);
    if (rc == M_C_FINISH || rc == M_C_NOTYET || rc == M_C_CLOSED) {
        if (mln_tcp_conn_get_head(&se->conn, M_C_RECV) != NULL) {
            start = c = mln_tcp_conn_remove(&se->conn, M_C_RECV);
            for (; c != NULL; c = c->next) {
                if (se->size_limit >= 0 && se->cur_size+mln_buf_size(c->buf) >= se->size_limit) {
                    if (se->size_limit > 0) {
                        last = c;
                        c = c->next;
                        last->buf->last = last->buf->pos + (se->size_limit - se->cur_size);
                    }
                    if (last != NULL) last->next = NULL;
                    mln_chain_pool_release_all(c);
                    se->cur_size = se->size_limit;
                    break;
                }
                last = c;
                se->cur_size += mln_buf_size(c->buf);
            }
            if (last != NULL) {
                if (se->head == NULL) {
                    se->head = start;
                    se->tail = last;
                } else {
                    se->tail->next = start;
                    se->tail = last;
                }
            }
        }
        if (rc == M_C_CLOSED) {
            if ((p = (mln_u8ptr_t)mln_alloc_m(ctx->pool, se->cur_size)) != NULL) {
                mln_string_nset(&s, p, se->cur_size);
                for (c = se->head; c != NULL; c = c->next) {
                    memcpy(p, c->buf->pos, mln_buf_size(c->buf));
                    p += mln_buf_size(c->buf);
                }
                ret_var = mln_lang_var_create_string(ctx, &s, NULL);
                mln_alloc_free(s.data);
                if (ret_var != NULL) {
                    mln_lang_ctx_set_ret_var(ctx, ret_var);
                }
            }
            mln_rbtree_delete(se->tree, se->rn);
            mln_rbtree_node_free(se->tree, se->rn);
            mln_lang_ctx_continue(ctx);
        }
    } else { /*M_C_ERROR*/
        mln_rbtree_delete(se->tree, se->rn);
        mln_rbtree_node_free(se->tree, se->rn);
        mln_lang_ctx_continue(ctx);
    }
}

#endif

