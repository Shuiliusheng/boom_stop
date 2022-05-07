#ifndef __INFO_H__
#define __INFO_H__

#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#define Cause_ExitSysCall 1
#define Cause_ExitInst 2


typedef struct{
    uint64_t addr;
    uint64_t size;
}MemRangeInfo;


extern bool showload;
extern MemRangeInfo data_seg, text_seg;

void takeoverSyscall();
uint64_t loadelf(char * progname, char *ckptinfo);
void read_ckptsyscall(char filename[]);
void read_ckptinfo(char ckptinfo[], char ckpt_sysinfo[]);

#define DEFINE_CSRR(s)                     \
    static inline uint64_t __csrr_##s()    \
    {                                      \
        uint64_t value;                    \
        __asm__ volatile("csrr    %0, " #s \
                         : "=r"(value)     \
                         :);               \
        return value;                      \
    }

DEFINE_CSRR(cycle)
DEFINE_CSRR(instret)

#endif
