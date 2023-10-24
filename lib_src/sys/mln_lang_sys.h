
/*
 * Copyright (C) Niklaus F.Schen.
 */
#ifndef __MLN_LANG_SYS_H
#define __MLN_LANG_SYS_H

#include "mln_lang.h"

typedef struct {
    mln_lang_ctx_t        *ctx;
    mln_tcp_conn_t         conn;
    mln_s64_t              size_limit;
    mln_s64_t              cur_size;
    mln_chain_t           *head;
    mln_chain_t           *tail;
    mln_rbtree_t          *tree;
    mln_rbtree_node_t     *rn;
    mln_s64_t              pid;
    mln_string_t          *cmd;
    mln_u32_t              running:1;
    mln_u32_t              padding:31;
} mln_lang_sys_exec_t;

struct mln_sys_diff_s {
    mln_lang_array_t      *dest;
    mln_lang_array_t      *notin;
    mln_u32_t              key;
};

typedef struct {
    mln_lang_ctx_t        *ctx;
    mln_fheap_node_t      *timer;
} mln_sys_msleep_t;

#endif

