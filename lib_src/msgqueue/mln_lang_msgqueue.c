
/*
 * Copyright (C) Niklaus F.Schen.
 */

#include "mln_lang_msgqueue.h"
#include "mln_conf.h"
#include "mln_log.h"

void destroy(mln_lang_t *lang);

static int mln_lang_msgqueue(mln_lang_ctx_t *ctx, mln_lang_object_t *obj)
{
    if (mln_lang_msgqueue_impl(ctx, obj) < 0) {
        destroy(ctx->lang);
        return -1;
    }
    return 0;
}

mln_lang_var_t *init(mln_lang_ctx_t *ctx, mln_conf_t *cf)
{
    mln_log_init(cf);
    mln_lang_var_t *obj = mln_lang_var_create_obj(ctx, NULL, NULL);
    if (obj == NULL) {
        mln_lang_errmsg(ctx, "No memory.");
        return NULL;
    }
    if (mln_lang_msgqueue(ctx, mln_lang_var_val_get(obj)->data.obj) < 0) {
        mln_lang_var_free(obj);
        return NULL;
    }
    return obj;
}

void destroy(mln_lang_t *lang)
{
    mln_rbtree_t *mq_set;
    mln_fheap_t *mq_timeout_set;
    if ((mq_set = mln_lang_resource_fetch(lang, "mq")) == NULL) {
        return;
    }
    if (!mln_rbtree_node_num(mq_set)) {
        mln_lang_resource_cancel(lang, "mq");
    }
    if ((mq_timeout_set = mln_lang_resource_fetch(lang, "mq_timeout")) == NULL) {
        return;
    }
    if (!mq_timeout_set->num) {
        mln_lang_resource_cancel(lang, "mq_timeout");
    }
}

