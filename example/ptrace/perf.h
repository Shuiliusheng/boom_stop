#ifndef   _PERF_H_  
#define   _PERF_H_

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <elf.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct user_regs_struct {
    unsigned long pc;
    unsigned long ra;
    unsigned long sp;
    unsigned long gp;
    unsigned long tp;
    unsigned long t0;
    unsigned long t1;
    unsigned long t2;
    unsigned long s0;
    unsigned long s1;
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
    unsigned long a4;
    unsigned long a5;
    unsigned long a6;
    unsigned long a7;
    unsigned long s2;
    unsigned long s3;
    unsigned long s4;
    unsigned long s5;
    unsigned long s6;
    unsigned long s7;
    unsigned long s8;
    unsigned long s9;
    unsigned long s10;
    unsigned long s11;
    unsigned long t3;
    unsigned long t4;
    unsigned long t5;
    unsigned long t6;
};

char **getargs(int argv, char **args);
FILE *read_params(const char filename[], uint64_t &eventsel, uint64_t &maxevent, uint64_t &maxperiods, uint64_t &warmupinst);

void start_run_and_trace(char pathname[], char **args);
void event_sample_process(uint64_t sampletimes, uint64_t exitpc);

extern uint64_t eventsel, maxevent, warmupinst, maxperiods;
extern char logname[256];


#endif