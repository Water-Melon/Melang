
/*
 * Copyright (C) Niklaus F.Schen.
 */
#include "mln_lang_json.h"
#include "mln_json.h"
#include "mln_utils.h"
#include "mln_conf.h"
#include "mln_log.h"

static int mln_lang_json(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static int mln_lang_json_encode_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static mln_lang_var_t *mln_lang_json_encode_process(mln_lang_ctx_t *ctx);
static inline int mln_lang_json_encode_generate(mln_lang_array_t *array, mln_json_t *json);
static int mln_lang_json_encode_generate_array(mln_lang_array_t *array, mln_json_t *json);
static inline int mln_lang_json_encode_generate_obj(mln_lang_array_t *array, mln_json_t *json);
static int mln_lang_json_encode_obj_iterate_handler(mln_rbtree_node_t *node, void *udata);

static int mln_lang_json_decode_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj);
static mln_lang_var_t *mln_lang_json_decode_process(mln_lang_ctx_t *ctx);
static inline int
mln_lang_json_decode_obj(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_rbtree_t *obj);
static inline int
mln_lang_json_decode_array(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_array_t *a);
static inline int
mln_lang_json_decode_string(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_string_t *s, mln_lang_var_t *key);
static inline int
mln_lang_json_decode_number(mln_lang_ctx_t *ctx, mln_lang_array_t *array, double num, mln_lang_var_t *key);
static inline int
mln_lang_json_decode_true(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key);
static inline int
mln_lang_json_decode_false(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key);
static inline int
mln_lang_json_decode_null(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key);
static int mln_lang_json_decode_obj_scan(mln_rbtree_node_t *node, void *data);


mln_lang_var_t *init(mln_lang_ctx_t *ctx, mln_conf_t *cf)
{
    mln_log_init(cf);
    mln_lang_var_t *obj = mln_lang_var_create_obj(ctx, NULL, NULL);
    if (obj == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (mln_lang_json(ctx, mln_lang_var_val_get(obj)->data.obj) < 0) {
        mln_lang_var_free(obj);
        return NULL;
    }
    return obj;
}

static int mln_lang_json(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    if (mln_lang_json_encode_handler(ctx, obj) < 0) return -1;
    if (mln_lang_json_decode_handler(ctx, obj) < 0) return -1;
    return 0;
}

static int mln_lang_json_encode_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("encode");
    mln_string_t v1 = mln_string("array");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_json_encode_process, NULL, NULL)) == NULL) {
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

static mln_lang_var_t *mln_lang_json_encode_process(mln_lang_ctx_t *ctx)
{
    mln_lang_val_t *val;
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("array");
    mln_lang_symbol_node_t *sym;
    mln_lang_array_t *array;
    mln_json_t json;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_ARRAY) {
        goto fout;
    }
    val = sym->data.var->val;
    if ((array = val->data.array) == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument.");
        return NULL;
    }

    if (mln_lang_json_encode_generate(array, &json) < 0) {
fout:
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
    } else {
        mln_string_t *s = mln_json_encode(&json);
        mln_json_destroy(&json);
        if (s == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        if ((ret_var = mln_lang_var_create_ref_string(ctx, s, NULL)) == NULL) {
            mln_string_free(s);
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
        mln_string_free(s);
    }

    return ret_var;
}

static inline int mln_lang_json_encode_generate(mln_lang_array_t *array, mln_json_t *json)
{
    if (!mln_rbtree_node_num(array->elems_key)) {
        return mln_lang_json_encode_generate_array(array, json);
    } else if (mln_rbtree_node_num(array->elems_index) == mln_rbtree_node_num(array->elems_key)) {
        return mln_lang_json_encode_generate_obj(array, json);
    }
    return -1;
}

static int mln_lang_json_encode_generate_array(mln_lang_array_t *array, mln_json_t *json)
{
    mln_s32_t type;
    mln_json_t j;
    mln_rbtree_node_t *rn;
    mln_lang_var_t *var;
    mln_lang_array_elem_t elem;
    int rc, i, n = mln_rbtree_node_num(array->elems_index);

    if (mln_json_array_init(json) < 0) {
        return -1;
    }

    for (i = 0; i < n; ++i) {
        elem.index = i;
        rn = mln_rbtree_search(array->elems_index, &elem);
        if (mln_rbtree_null(rn, array->elems_index)) {
            mln_json_destroy(json);
            return -1;
        }
        var = ((mln_lang_array_elem_t *)mln_rbtree_node_data_get(rn))->value;
        type = mln_lang_var_val_type_get(var);
        switch (type) {
            case M_LANG_VAL_TYPE_NIL:
                mln_json_null_init(&j);
                break;
            case M_LANG_VAL_TYPE_BOOL:
                if (var->val->data.b) mln_json_true_init(&j);
                else mln_json_false_init(&j);
                break;
            case M_LANG_VAL_TYPE_INT:
                mln_json_number_init(&j, var->val->data.i);
                break;
            case M_LANG_VAL_TYPE_REAL:
                mln_json_number_init(&j, var->val->data.f);
                break;
            case M_LANG_VAL_TYPE_STRING:
                if (var->val->data.s == NULL) {
                    mln_json_destroy(json);
                    return -1;
                }
                mln_json_string_init(&j, mln_string_ref(var->val->data.s));
                break;
            case M_LANG_VAL_TYPE_OBJECT:
            case M_LANG_VAL_TYPE_FUNC:
                mln_json_destroy(json);
                return -1;
            case M_LANG_VAL_TYPE_ARRAY:
            {
                mln_lang_array_t *tmpa;
                if ((tmpa = var->val->data.array) == NULL) {
                    mln_json_destroy(json);
                    return -1;
                }
                rc = -1;
                if (!mln_rbtree_node_num(tmpa->elems_key)) {
                    rc = mln_lang_json_encode_generate_array(tmpa, &j);
                } else if (mln_rbtree_node_num(tmpa->elems_index) == mln_rbtree_node_num(tmpa->elems_key)) {
                    rc = mln_lang_json_encode_generate_obj(tmpa, &j);
                }
                if (rc < 0) {
                    mln_json_destroy(json);
                    return -1;
                }
                break;
            }
            default:
                ASSERT(0);
                break;
        }
        if (mln_json_is_none(&j)) continue;

        if (mln_json_array_append(json, &j) < 0) {
            mln_json_destroy(&j);
            mln_json_destroy(json);
            return -1;
        }
    }
    return 0;
}

static inline int mln_lang_json_encode_generate_obj(mln_lang_array_t *array, mln_json_t *json)
{
    if (mln_json_obj_init(json) < 0) {
        return -1;
    }
    if (mln_rbtree_iterate(array->elems_key, \
                            mln_lang_json_encode_obj_iterate_handler, \
                            json) < 0)
    {
        mln_json_destroy(json);
        return -1;
    }
    return 0;
}

static int mln_lang_json_encode_obj_iterate_handler(mln_rbtree_node_t *node, void *udata)
{
    mln_s32_t type;
    mln_string_t *k;
    mln_lang_var_t *var;
    mln_json_t *jparent = (mln_json_t *)udata, j, kj;
    mln_lang_array_elem_t *lae = (mln_lang_array_elem_t *)mln_rbtree_node_data_get(node);

    if (mln_lang_var_val_type_get(lae->key) != M_LANG_VAL_TYPE_STRING || (k = lae->key->val->data.s) == NULL)
        return 0;

    var = lae->value;
    type = mln_lang_var_val_type_get(var);
    switch (type) {
        case M_LANG_VAL_TYPE_NIL:
            mln_json_null_init(&j);
            break;
        case M_LANG_VAL_TYPE_BOOL:
            if (var->val->data.b) mln_json_true_init(&j);
            else mln_json_false_init(&j);
            break;
        case M_LANG_VAL_TYPE_INT:
            mln_json_number_init(&j, var->val->data.i);
            break;
        case M_LANG_VAL_TYPE_REAL:
            mln_json_number_init(&j, var->val->data.f);
            break;
        case M_LANG_VAL_TYPE_STRING:
            if (var->val->data.s == NULL) return -1;
            mln_json_string_init(&j, mln_string_ref(var->val->data.s));
            break;
        case M_LANG_VAL_TYPE_OBJECT:
        case M_LANG_VAL_TYPE_FUNC:
            return -1;
        case M_LANG_VAL_TYPE_ARRAY:
        {
            mln_lang_array_t *tmpa;
            if ((tmpa = var->val->data.array) == NULL) {
                return -1;
            }
            int rc = -1;
            if (!mln_rbtree_node_num(tmpa->elems_key)) {
                rc = mln_lang_json_encode_generate_array(tmpa, &j);
            } else if (mln_rbtree_node_num(tmpa->elems_index) == mln_rbtree_node_num(tmpa->elems_key)) {
                rc = mln_lang_json_encode_generate_obj(tmpa, &j);
            }
            if (rc < 0) return -1;
            break;
        }
        default:
            ASSERT(0);
            break;
    }
    if (mln_json_is_none(&j)) return 0;

    mln_json_string_init(&kj, mln_string_ref(k));
    if (mln_json_obj_update(jparent, &kj, &j) < 0) {
        mln_json_destroy(&kj);
        mln_json_destroy(&j);
        return -1;
    }
    return 0;
}


static int mln_lang_json_decode_handler(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    mln_lang_val_t *val;
    mln_lang_var_t *var;
    mln_lang_func_detail_t *func;
    mln_string_t funcname = mln_string("decode");
    mln_string_t v1 = mln_string("s");
    if ((func = mln_lang_func_detail_new(ctx, M_FUNC_INTERNAL, mln_lang_json_decode_process, NULL, NULL)) == NULL) {
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

static mln_lang_var_t *mln_lang_json_decode_process(mln_lang_ctx_t *ctx)
{
    int rc = 0;
    mln_lang_val_t *val;
    mln_lang_var_t *ret_var;
    mln_string_t v1 = mln_string("s");
    mln_lang_symbol_node_t *sym;
    mln_lang_array_t *array;
    mln_json_t json;

    if ((sym = mln_lang_symbol_node_search(ctx, &v1, 1)) == NULL) {
        ASSERT(0);
        mln_lang_errmsg(ctx, "Argument missing.");
        return NULL;
    }
    ASSERT(sym->type == M_LANG_SYMBOL_VAR);
    if (mln_lang_var_val_type_get(sym->data.var) != M_LANG_VAL_TYPE_STRING) {
        mln_lang_errmsg(ctx, "Invalid type of argument.");
        return NULL;
    }
    val = sym->data.var->val;
    if (val->data.s == NULL) {
        mln_lang_errmsg(ctx, "Invalid argument.");
        return NULL;
    }
    if ((ret_var = mln_lang_var_create_array(ctx, NULL)) == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    array = ret_var->val->data.array;

    if (mln_json_decode(val->data.s, &json, NULL) < 0) {
        mln_lang_var_free(ret_var);
        if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
            mln_lang_errmsg(ctx, "No memory.");
            return NULL;
        }
    } else {
        if (mln_json_is_object(&json)) {
            rc = mln_lang_json_decode_obj(ctx, array, mln_json_object_data_get(&json));
        } else if (mln_json_is_array(&json)) {
            rc = mln_lang_json_decode_array(ctx, array, mln_json_array_data_get(&json));
        } else if (mln_json_is_string(&json)) {
            rc = mln_lang_json_decode_string(ctx, array, mln_json_string_data_get(&json), NULL);
        } else if (mln_json_is_number(&json)) {
            rc = mln_lang_json_decode_number(ctx, array, mln_json_number_data_get(&json), NULL);
        } else if (mln_json_is_true(&json)) {
            rc = mln_lang_json_decode_true(ctx, array, NULL);
        } else if (mln_json_is_false(&json)) {
            rc = mln_lang_json_decode_false(ctx, array, NULL);
        } else if (mln_json_is_null(&json)) {
            rc = mln_lang_json_decode_null(ctx, array, NULL);
        } else { /*M_JSON_IS_NONE*/
            /*do nothing*/
        }
        mln_json_destroy(&json);
        if (rc < 0) {
            mln_lang_var_free(ret_var);
            if ((ret_var = mln_lang_var_create_false(ctx, NULL)) == NULL) {
                mln_lang_errmsg(ctx, "No memory.");
                return NULL;
            }
        }
    }
    return ret_var;
}

struct mln_lang_json_scan_s {
    mln_lang_ctx_t   *ctx;
    mln_lang_array_t *array;
};

static int
mln_lang_json_decode_obj(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_rbtree_t *obj)
{
    struct mln_lang_json_scan_s ljs;
    ljs.ctx = ctx;
    ljs.array = array;
    if (mln_rbtree_iterate(obj, mln_lang_json_decode_obj_scan, &ljs) < 0) {
        return -1;
    }
    return 0;
}

static int mln_lang_json_decode_obj_scan(mln_rbtree_node_t *node, void *data)
{
    int rc = 0;
    mln_json_kv_t *kv = (mln_json_kv_t *)mln_rbtree_node_data_get(node);
    struct mln_lang_json_scan_s *ljs = (struct mln_lang_json_scan_s *)data;
    mln_lang_var_t kvar;
    mln_lang_val_t kval;
    kvar.type = M_LANG_VAR_NORMAL;
    kvar.name = NULL;
    kvar.val = &kval;
    kvar.in_set = NULL;
    kvar.prev = kvar.next = NULL;
    kval.data.s = mln_json_string_data_get(&(kv->key));
    kval.type = M_LANG_VAL_TYPE_STRING;
    kval.ref = 1;

    if (mln_json_is_object(&(kv->val))) {
        mln_lang_array_t *tmpa;
        mln_lang_var_t *array_val, var;
        mln_lang_val_t val;
        if ((tmpa = mln_lang_array_new(ljs->ctx)) == NULL) {
            return -1;
        }
        ++(tmpa->ref);
        if ((rc = mln_lang_json_decode_obj(ljs->ctx, tmpa, mln_json_object_data_get(&(kv->val)))) < 0) {
            mln_lang_array_free(tmpa);
            return rc;
        }
        if ((array_val = mln_lang_array_get(ljs->ctx, ljs->array, &kvar)) == NULL) {
            mln_lang_array_free(tmpa);
            return -1;
        }
        var.type = M_LANG_VAR_NORMAL;
        var.name = NULL;
        var.val = &val;
        var.in_set = NULL;
        var.prev = var.next = NULL;
        val.data.array = tmpa;
        val.type = M_LANG_VAL_TYPE_ARRAY;
        val.ref = 1;
        if (mln_lang_var_value_set(ljs->ctx, array_val, &var) < 0) {
            mln_lang_array_free(tmpa);
            return -1;
        }
    } else if (mln_json_is_array(&(kv->val))) {
        mln_lang_array_t *tmpa;
        mln_lang_var_t *array_val, var;
        mln_lang_val_t val;
        if ((tmpa = mln_lang_array_new(ljs->ctx)) == NULL) {
            return -1;
        }
        ++(tmpa->ref);
        if ((rc = mln_lang_json_decode_array(ljs->ctx, tmpa, mln_json_array_data_get(&(kv->val)))) < 0) {
            mln_lang_array_free(tmpa);
            return rc;
        }
        if ((array_val = mln_lang_array_get(ljs->ctx, ljs->array, &kvar)) == NULL) {
            mln_lang_array_free(tmpa);
            return -1;
        }
        var.type = M_LANG_VAR_NORMAL;
        var.name = NULL;
        var.val = &val;
        var.in_set = NULL;
        var.prev = var.next = NULL;
        val.data.array = tmpa;
        val.type = M_LANG_VAL_TYPE_ARRAY;
        val.ref = 1;
        if (mln_lang_var_value_set(ljs->ctx, array_val, &var) < 0) {
            mln_lang_array_free(tmpa);
            return -1;
        }
    } else if (mln_json_is_string(&(kv->val))) {
        rc = mln_lang_json_decode_string(ljs->ctx, ljs->array, mln_json_string_data_get(&(kv->val)), &kvar);
    } else if (mln_json_is_number(&(kv->val))) {
        rc = mln_lang_json_decode_number(ljs->ctx, ljs->array, mln_json_number_data_get(&(kv->val)), &kvar);
    } else if (mln_json_is_true(&(kv->val))) {
        rc = mln_lang_json_decode_true(ljs->ctx, ljs->array, &kvar);
    } else if (mln_json_is_false(&(kv->val))) {
        rc = mln_lang_json_decode_false(ljs->ctx, ljs->array, &kvar);
    } else if (mln_json_is_null(&(kv->val))) {
        rc = mln_lang_json_decode_null(ljs->ctx, ljs->array, &kvar);
    } else { /*M_JSON_IS_NONE*/
        /*do nothing*/
    }

    return rc;
}

static int
mln_lang_json_decode_array(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_array_t *a)
{
    mln_u64_t i;
    mln_lang_var_t kvar;
    mln_lang_val_t kval;
    mln_lang_array_t *tmpa;
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    mln_json_t *el = (mln_json_t *)mln_array_elts(a);
    mln_json_t *elend = el + mln_array_nelts(a);

    for (i = 0; el < elend; ++el) {
        kvar.type = M_LANG_VAR_NORMAL;
        kvar.name = NULL;
        kvar.val = &kval;
        kvar.in_set = NULL;
        kvar.prev = kvar.next = NULL;
        kval.data.i = i++;
        kval.type = M_LANG_VAL_TYPE_INT;
        kval.ref = 1;

        if (mln_json_is_object(el)) {
            if ((tmpa = mln_lang_array_new(ctx)) == NULL) {
                return -1;
            }
            ++(tmpa->ref);
            if (mln_lang_json_decode_obj(ctx, tmpa, mln_json_object_data_get(el)) < 0) {
                mln_lang_array_free(tmpa);
                return -1;
            }
            if ((array_val = mln_lang_array_get(ctx, array, &kvar)) == NULL) {
                mln_lang_array_free(tmpa);
                return -1;
            }
            var.type = M_LANG_VAR_NORMAL;
            var.name = NULL;
            var.val = &val;
            var.in_set = NULL;
            var.prev = var.next = NULL;
            val.data.array = tmpa;
            val.type = M_LANG_VAL_TYPE_ARRAY;
            val.ref = 1;
            if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
                mln_lang_array_free(tmpa);
                return -1;
            }
        } else if (mln_json_is_array(el)) {
            if ((tmpa = mln_lang_array_new(ctx)) == NULL) {
                return -1;
            }
            ++(tmpa->ref);
            if (mln_lang_json_decode_array(ctx, tmpa, mln_json_array_data_get(el)) < 0) {
                mln_lang_array_free(tmpa);
                return -1;
            }
            if ((array_val = mln_lang_array_get(ctx, array, &kvar)) == NULL) {
                mln_lang_array_free(tmpa);
                return -1;
            }
            var.type = M_LANG_VAR_NORMAL;
            var.name = NULL;
            var.val = &val;
            var.in_set = NULL;
            var.prev = var.next = NULL;
            val.data.array = tmpa;
            val.type = M_LANG_VAL_TYPE_ARRAY;
            val.ref = 1;
            if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
                mln_lang_array_free(tmpa);
                return -1;
            }
        } else if (mln_json_is_string(el)) {
            if (mln_lang_json_decode_string(ctx, array, mln_json_string_data_get(el), &kvar) < 0)
                return -1;
        } else if (mln_json_is_number(el)) {
            if (mln_lang_json_decode_number(ctx, array, mln_json_number_data_get(el), &kvar) < 0)
                return -1;
        } else if (mln_json_is_true(el)) {
            if (mln_lang_json_decode_true(ctx, array, &kvar) < 0)
                return -1;
        } else if (mln_json_is_false(el)) {
            if (mln_lang_json_decode_false(ctx, array, &kvar) < 0)
                return -1;
        } else if (mln_json_is_null(el)) {
            if (mln_lang_json_decode_null(ctx, array, &kvar) < 0)
                return -1;
        } else { /*M_JSON_IS_NONE*/
            /*do nothing*/
        }
    }
    return 0;
}

static inline int
mln_lang_json_decode_string(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_string_t *s, mln_lang_var_t *key)
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

static inline int
mln_lang_json_decode_number(mln_lang_ctx_t *ctx, mln_lang_array_t *array, double num, mln_lang_var_t *key)
{
    mln_lang_var_t *array_val, var;
    mln_lang_val_t val;
    mln_s64_t i = (mln_s64_t)num;
    if ((array_val = mln_lang_array_get(ctx, array, key)) == NULL) {
        return -1;
    }
    var.type = M_LANG_VAR_NORMAL;
    var.name = NULL;
    var.val = &val;
    var.in_set = NULL;
    var.prev = var.next = NULL;
    if (i == num) {
        val.data.i = num;
        val.type = M_LANG_VAL_TYPE_INT;
    } else {
        val.data.f = num;
        val.type = M_LANG_VAL_TYPE_REAL;
    }
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static inline int
mln_lang_json_decode_true(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key)
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
    val.data.b = 1;
    val.type = M_LANG_VAL_TYPE_BOOL;
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static inline int
mln_lang_json_decode_false(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key)
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
    val.data.b = 0;
    val.type = M_LANG_VAL_TYPE_BOOL;
    val.ref = 1;
    if (mln_lang_var_value_set(ctx, array_val, &var) < 0) {
        return -1;
    }
    return 0;
}

static inline int
mln_lang_json_decode_null(mln_lang_ctx_t *ctx, mln_lang_array_t *array, mln_lang_var_t *key)
{
    return mln_lang_array_get(ctx, array, key) == NULL? -1: 0;
}

