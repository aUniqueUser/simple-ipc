/*
 * util.cpp
 *
 *  Created on: Mar 19, 2017
 *      Author: nullifiedcat
 */

#include "linux/util.h"

#include <stdio.h>

int read_stat(pid_t pid, struct proc_stat_s *s) {
    static const char * const procfile = "/proc/%d/stat";
    static const char * const format = "%d %s %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %lu %lu %llu";

    char buf[128];
    sprintf(buf, procfile, pid);

    FILE *proc = fopen(buf, "r");
    if(proc)
    {
        int ret = fscanf(proc, format,
                         &s->pid,
                         s->comm,
                         &s->state,
                         &s->ppid,
                         &s->pgrp,
                         &s->session,
                         &s->tty_nr,
                         &s->tpgid,
                         &s->flags,
                         &s->minflt,
                         &s->cminflt,
                         &s->majflt,
                         &s->cmajflt,
                         &s->utime,
                         &s->stime,
                         &s->cutime,
                         &s->cstime,
                         &s->priority,
                         &s->nice,
                         &s->num_threads,
                         &s->itrealvalue,
                         &s->starttime,
                         &s->vsize,
                         &s->rss,
                         &s->rlim,
                         &s->startcode,
                         &s->endcode,
                         &s->startstack,
                         &s->kstkesp,
                         &s->kstkeip,
                         &s->signal,
                         &s->blocked,
                         &s->sigignore,
                         &s->sigcatch,
                         &s->wchan,
                         &s->nswap,
                         &s->cnswap,
                         &s->exit_signal,
                         &s->processor,
                         &s->rt_priority,
                         &s->policy,
                         &s->delayacct_blkio_ticks);
        fclose(proc);
        if(ret == 42) return 1;
    }
    return 0;
}
