/*
 * Copyright (C) Niklaus F.Schen.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "mln_core.h"
#include "mln_string.h"
#include "mln_lang.h"
#include "mln_lex.h"
#include "mln_log.h"
#include "mln_conf.h"

static int mln_global_init(void);
static void mln_params_check(int argc, char *argv[]);
static void mln_run_all(int argc, char *argv[]);
static void taskChecker(mln_event_t *ev, void *data);

int main(int argc, char *argv[])
{
    mln_params_check(argc, argv);

    struct mln_core_attr cattr;
    cattr.argc = argc;
    cattr.argv = argv;
    cattr.global_init = mln_global_init;
    cattr.worker_process = NULL;
    if (mln_core_init(&cattr) < 0) {
        fprintf(stderr, "init failed.\n");
        return -1;
    }

    mln_run_all(argc, argv);

    return 0;
}

static int mln_global_init(void)
{
    mln_string_t path = mln_string("/tmp/.melang.log");
    mln_conf_t *cf = mln_get_conf();
    mln_conf_domain_t *cd = cf->search(cf, "main");
    mln_conf_cmd_t *cc;
    mln_conf_item_t *ci;

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

    return 0;
}

static void mln_params_check(int argc, char *argv[])
{
    int i = 1, fd, nofile = 1;
    for (; i < argc; ++i) {
        if (!strcmp(argv[i], "-v")) {
            printf("Melang 0.3.0.\n");
        } else if (!strcmp(argv[i], "-h")) {
            printf("Melang Usage:\n");
            printf("\tmelang <script-file> ...\n");
            printf("\t-v show version\n");
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

static void mln_run_all(int argc, char *argv[])
{
    mln_alloc_t *pool = mln_alloc_init();
    if (pool == NULL) {
        mln_log(error, "Pool init failed.\n");
        exit(1);
    }
    mln_event_t *ev = mln_event_init(1);
    if (ev == NULL) {
	mln_log(error, "ev init failed.\n");
        exit(1);
    }
    mln_lang_t *lang = mln_lang_new(pool, ev);
    if (lang == NULL) {
	mln_log(error, "lang init failed.\n");
        exit(1);
    }
    mln_string_t path;
    int i;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "-h")) {
            continue;
        }
        mln_string_nSet(&path, argv[i], strlen(argv[i]));
        mln_lang_job_new(lang, M_INPUT_T_FILE, &path, NULL, NULL);
    }
    mln_lang_run(lang);

    if (mln_event_set_timer(ev, 100, lang, taskChecker) < 0) {
        mln_log(error, "Set timer failed.\n");
        exit(1);
    }

    mln_event_dispatch(ev);
}

static void taskChecker(mln_event_t *ev, void *data)
{
    mln_lang_t *lang = (mln_lang_t *)data;
    if (lang->run_head == NULL && lang->blocked_head == NULL && lang->wait_head == NULL) {
        mln_event_set_break(ev);
        return;
    }
    mln_event_set_timer(ev, 100, lang, taskChecker);
}

