/*
 * Copyright (C) Niklaus F.Schen.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "mln_core.h"
#include "mln_string.h"
#include "mln_lang.h"
#include "mln_lex.h"
#include "mln_log.h"
#include "mln_conf.h"
#include "mln_iothread.h"
#include "mln_rbtree.h"

typedef struct mln_fd_node_s {
    pthread_t             tid;
    int                   signal_fd;
    int                   unused_fd;
    struct mln_fd_node_s *prev;
    struct mln_fd_node_s *next;
} mln_fd_node_t;

struct mln_thread_args_s {
    mln_event_t          *ev;
    mln_iothread_t       *t;
};

static int mln_signal(mln_lang_t *lang);
static int mln_clear(mln_lang_t *lang);
static int mln_global_init(void);
static void mln_params_check(int argc, char *argv[]);
static void mln_run_all(int argc, char *argv[]);
static void mln_task_checker(mln_event_t *ev, void *data);
static mln_fd_node_t *mln_fd_node_new(void);
static void mln_fd_node_free(mln_fd_node_t *n);
static int mln_fd_node_cmp(const mln_fd_node_t *n1, const mln_fd_node_t *n2);
static void mln_iothread_msg_handler(mln_iothread_t *t, mln_iothread_ep_type_t from, mln_iothread_msg_t *msg);
MLN_CHAIN_FUNC_DECLARE(mln_fd_node, mln_fd_node_t, static inline void,);
MLN_CHAIN_FUNC_DEFINE(mln_fd_node, mln_fd_node_t, static inline void, prev, next);

static mln_rbtree_t *fd_tree = NULL;
static mln_fd_node_t *head = NULL;
static mln_fd_node_t *tail = NULL;
static pthread_mutex_t lock;
__thread mln_fd_node_t *t_node;

int main(int argc, char *argv[])
{
#if defined(WIN32)
    WSADATA wsaData = {0};
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc < 0) {
        fprintf(stderr, "WSAStartup faiiled. %d\n", WSAGetLastError());
    return -1;
    }
#endif
    mln_params_check(argc, argv);

    struct mln_core_attr cattr;
    cattr.argc = argc;
    cattr.argv = argv;
    cattr.global_init = mln_global_init;
#if !defined(WIN32)
    cattr.worker_process = NULL;
    cattr.master_process = NULL;
#endif
    if (mln_core_init(&cattr) < 0) {
        fprintf(stderr, "init failed.\n");
        return -1;
    }

    mln_run_all(argc, argv);

#if defined(WIN32)
    WSACleanup();
#endif
    return 0;
}

static int mln_global_init(void)
{
    mln_string_t path = mln_string("/tmp/.melang.log");
    mln_conf_t *cf = mln_get_conf();
    mln_conf_domain_t *cd = cf->search(cf, "main");
    mln_conf_cmd_t *cc;
    mln_conf_item_t *ci;
    struct mln_rbtree_attr rbattr;

    cc = cd->search(cd, "daemon");
    if (cc == NULL) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    ci = cc->search(cc, 1);
    if (ci == NULL || ci->type != CONF_BOOL) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    ci->val.b = 0;

    cc = cd->search(cd, "framework");
    if (cc == NULL) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    ci = cc->search(cc, 1);
    if (ci == NULL || ci->type != CONF_BOOL) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    ci->val.b = 0;

    cc = cd->search(cd, "worker_proc");
    if (cc == NULL) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    ci = cc->search(cc, 1);
    if (ci == NULL || ci->type != CONF_INT) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    ci->val.i = 1;

    cc = cd->search(cd, "log_path");
    if (cc == NULL) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    ci = cc->search(cc, 1);
    if (ci == NULL || ci->type != CONF_STR) {
        fprintf(stderr, "Invalid configuration.\n");
        return -1;
    }
    mln_string_free(ci->val.s);
    if ((ci->val.s = mln_string_dup(&path)) == NULL) {
        fprintf(stderr, "No memory.\n");
        return -1;
    }

    rbattr.pool = NULL;
    rbattr.pool_alloc = NULL;
    rbattr.pool_free = NULL;
    rbattr.cmp = (rbtree_cmp)mln_fd_node_cmp;
    rbattr.data_free = (rbtree_free_data)mln_fd_node_free;
    rbattr.cache = 0;
    if ((fd_tree = mln_rbtree_init(&rbattr)) == NULL) {
        fprintf(stderr, "Init fd tree failed.\n");
        return -1;
    }

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "Init fd tree mutex failed.\n");
        return -1;
    }
    return 0;
}

static void mln_params_check(int argc, char *argv[])
{
    int i = 1, fd, nofile = 1;
    for (; i < argc; ++i) {
        if (!strcmp(argv[i], "-v")) {
            printf("Melang 0.6.0.\n");
        } else if (!strcmp(argv[i], "-h")) {
            printf("Melang Usage:\n");
            printf("\tmelang <script-file> ...\n");
            printf("\t-v\t\t\tshow version\n");
            printf("\t-t=number_of_threads\tspecify the number of threads. Default is 1\n");
        } else if (!strncmp(argv[i], "-t=", 3)) {
            /* do nothing */
        } else {
            if ((fd = open(argv[i], O_RDONLY)) < 0) {
                fprintf(stderr, "File %s cannot be read, %s\n", argv[i], strerror(errno));
                exit(1);
            }
            close(fd);
            nofile = 0;
        }
    }
    if (nofile) {
        exit(0);
    }
}

static void *mln_iothread_entry(void *args)
{
    int rc;
    struct mln_thread_args_s *a = (struct mln_thread_args_s *)args;
    mln_rbtree_node_t *rn;
    mln_fd_node_t *n = mln_fd_node_new();
    if (n == NULL) {
        mln_log(error, "No memory\n");
        return NULL;
    }
    t_node = n;

    pthread_mutex_lock(&lock);

    mln_fd_node_chain_add(&head, &tail, n);

    if ((rn = mln_rbtree_node_new(fd_tree, n)) == NULL) {
        pthread_mutex_unlock(&lock);
        mln_log(error, "No memory\n");
        return NULL;
    }
    mln_rbtree_insert(fd_tree, rn);

    pthread_mutex_unlock(&lock);

    while (!(rc = mln_iothread_recv(a->t, user_thread)))
        ;
    pthread_mutex_lock(&lock);
    mln_fd_node_chain_del(&head, &tail, n);
    pthread_mutex_unlock(&lock);

    mln_event_dispatch(a->ev);

    return NULL;
}

static void mln_iothread_msg_handler(mln_iothread_t *t, mln_iothread_ep_type_t from, mln_iothread_msg_t *msg)
{
    mln_lang_t *lang = (mln_lang_t *)mln_iothread_msg_data(msg);
    mln_lang_signal_get(lang)(lang);
}

static void mln_run_all(int argc, char *argv[])
{
    int i, nth = 1;
    mln_iothread_t t;
    mln_string_t path;
    mln_event_t *ev;
    mln_lang_t *lang;
    struct mln_iothread_attr tattr;
    mln_rbtree_node_t *rn;
    mln_fd_node_t *n;
    struct mln_thread_args_s args;

    if ((ev = mln_event_new()) == NULL) {
        mln_log(error, "ev init failed.\n");
        exit(1);
    }

    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "-h")) {
            continue;
        } else if (!strncmp(argv[i], "-t=", 3)) {
            nth = atoi(&argv[i][3]);
            break;
        }
    }

    args.ev = ev;
    args.t = &t;

    if (nth > 1) {
        tattr.nthread = nth - 1;
        tattr.entry = (mln_iothread_entry_t)mln_iothread_entry;
        tattr.args = &args;
        tattr.handler = mln_iothread_msg_handler;
        if (mln_iothread_init(&t, &tattr) < 0) {
            mln_log(error, "iothread init failed.\n");
            exit(1);
        }
    }


    n = mln_fd_node_new();
    if (n == NULL) {
        mln_log(error, "No memory\n");
        exit(1);
    }
    t_node = n;

    pthread_mutex_lock(&lock);

    if ((rn = mln_rbtree_node_new(fd_tree, n)) == NULL) {
        pthread_mutex_unlock(&lock);
        mln_log(error, "No memory\n");
        exit(1);
    }
    mln_rbtree_insert(fd_tree, rn);

    pthread_mutex_unlock(&lock);


    if ((lang = mln_lang_new(ev, mln_signal, mln_clear)) == NULL) {
        mln_log(error, "lang init failed.\n");
        exit(1);
    }
    mln_lang_cache_set(lang);

    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "-h") || !strncmp(argv[i], "-t=", 3)) {
            continue;
        }
        mln_string_nset(&path, argv[i], strlen(argv[i]));
        mln_lang_job_new(lang, M_INPUT_T_FILE, &path, NULL, NULL);
    }

    if (mln_event_timer_set(ev, 1, lang, mln_task_checker) < 0) {
        mln_log(error, "Set timer failed.\n");
        exit(1);
    }

    if (nth > 1) {
        while (1) {
            pthread_mutex_lock(&lock);
            if (head != NULL) {
                pthread_mutex_unlock(&lock);
                mln_iothread_send(&t, 0, lang, io_thread, 0);
            } else {
                pthread_mutex_unlock(&lock);
                break;
            }
        }
    }

    mln_event_dispatch(ev);
}

static void mln_task_checker(mln_event_t *ev, void *data)
{
    mln_lang_t *lang = (mln_lang_t *)data;
    mln_lang_mutex_lock(lang);
    if (mln_lang_task_empty(lang)) {
        mln_lang_mutex_unlock(lang);
        mln_event_break_set(ev);
        return;
    }
    mln_lang_signal_get(lang)(lang);
    mln_lang_mutex_unlock(lang);
    mln_event_timer_set(ev, 1, lang, mln_task_checker);
}

static int mln_signal(mln_lang_t *lang)
{
    return mln_event_fd_set(mln_lang_event_get(lang), t_node->signal_fd, M_EV_SEND|M_EV_ONESHOT, M_EV_UNLIMITED, lang, mln_lang_launcher_get(lang));
}

static int mln_clear(mln_lang_t *lang)
{
    return mln_event_fd_set(mln_lang_event_get(lang), t_node->signal_fd, M_EV_CLR, M_EV_UNLIMITED, NULL, NULL);
}

/*
 * mln_fd_node_t
 */
static mln_fd_node_t *mln_fd_node_new(void)
{
    int fds[2];
    mln_fd_node_t *n = (mln_fd_node_t *)malloc(sizeof(mln_fd_node_t));
    if (n == NULL) return NULL;
    n->tid = pthread_self();
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        free(n);
        return NULL;
    }
    n->signal_fd = fds[0];
    n->unused_fd = fds[1];
    n->prev = n->next = NULL;
    return n;
}

static void mln_fd_node_free(mln_fd_node_t *n)
{
    if (n == NULL) return;

    mln_socket_close(n->signal_fd);
    mln_socket_close(n->unused_fd);
    free(n);
}

static int mln_fd_node_cmp(const mln_fd_node_t *n1, const mln_fd_node_t *n2)
{
    if (n1->tid > n2->tid) return 1;
    if (n1->tid < n2->tid) return -1;
    return 0;
}

