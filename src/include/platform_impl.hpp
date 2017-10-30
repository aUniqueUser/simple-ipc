/*
 * platform.cpp
 *
 *  Created on: Oct 24, 2017
 *      Author: nullifiedcat
 */

#include "iipc.hpp"

#if defined(__linux__)

#   include <sys/types.h>
#   include <unistd.h>
#   include <stdio.h>

namespace cat_ipc
{

struct proc_stat_s {
    int pid; // %d
    char comm[256]; // %s
    char state; // %c
    int ppid; // %d
    int pgrp; // %d
    int session; // %d
    int tty_nr; // %d
    int tpgid; // %d
    unsigned long flags; // %lu OR %l
    unsigned long minflt; // %lu
    unsigned long cminflt; // %lu
    unsigned long majflt; // %lu
    unsigned long cmajflt; // %lu
    unsigned long utime; // %lu
    unsigned long stime;  // %lu
    long cutime; // %ld
    long cstime; // %ld
    long priority; // %ld
    long nice; // %ld
    long num_threads; // %ld
    long itrealvalue; // %ld
    unsigned long starttime; // %lu
    unsigned long vsize; // %lu
    long rss; // %ld
    unsigned long rlim; // %lu
    unsigned long startcode; // %lu
    unsigned long endcode; // %lu
    unsigned long startstack; // %lu
    unsigned long kstkesp; // %lu
    unsigned long kstkeip; // %lu
    unsigned long signal; // %lu
    unsigned long blocked; // %lu
    unsigned long sigignore; // %lu
    unsigned long sigcatch; // %lu
    unsigned long wchan; // %lu
    unsigned long nswap; // %lu
    unsigned long cnswap; // %lu
    int exit_signal; // %d
    int processor; // %d
    unsigned long rt_priority; // %lu
    unsigned long policy; // %lu
    unsigned long long delayacct_blkio_ticks; // %llu
};

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

void _PLATFORM_ xstoredata(cat_ipc::internal::client_data& data)
{
    proc_stat_s stat;
    read_stat(getpid(), &stat);
    data.process = getpid();
    data.start_time = stat.starttime;
}

bool _PLATFORM_ xcheckdead(cat_ipc::internal::client_data& data)
{
    proc_stat_s stat;
    if (0 == read_stat(data.process, &stat))
    {
        return true;
    }
    return data.start_time != stat.starttime;
}

}

#elif defined(WIN32)

#   include <Windows.h>

namespace cat_ipc
{

void _PLATFORM_ xstoredata(cat_ipc::internal::client_data& data)
{
    HANDLE current = GetCurrentProcess();
    data.process = GetProcessId(current);
    FILETIME ftExit, ftKernel, ftUser;
    GetProcessTimes(current, &data.start_time, &ftExit, &ftKernel, &ftUser);
}

bool _PLATFORM_ xcheckdead(cat_ipc::internal::client_data& data)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, data.process);
    if (process == NULL)
    {
        return true;
    }
    FILETIME ft, ftExit, ftKernel, ftUser;
    GetProcessTimes(process, &ft, &ftExit, &ftKernel, &ftUser);
    return ft.dwLowDateTime != data.start_time.dwLowDateTime ||
           ft.dwHighDateTime != data.start_time.dwHighDateTime;
}

}

#endif