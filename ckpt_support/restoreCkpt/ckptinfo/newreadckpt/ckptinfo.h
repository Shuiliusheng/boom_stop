#ifndef __INFO_H__
#define __INFO_H__

#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

typedef struct{
    uint64_t addr;
    uint32_t inst;
}JmpInfo;

typedef struct{
    int num;
    JmpInfo *infos;
}JmpRepInfo;

typedef struct{
    uint64_t addr;
    uint64_t size;
}MemRangeInfo;

typedef struct{
    uint64_t pc;
    uint64_t num;
    uint64_t p0, p1, p2;
    uint64_t hasret, ret;
    uint64_t bufaddr, data_offset, data_size;
}SyscallInfo;

typedef struct{
    uint64_t exitpc;
    uint64_t exit_cause;
    uint64_t syscall_info_addr;
    uint64_t nowcallnum;
    uint64_t totalcallnum;
    uint64_t intregs[32];
    uint64_t fpregs[32];
    uint64_t oldregs[32];
    uint64_t cycles;
    uint64_t insts;
    uint64_t lastcycles;
    uint64_t lastinsts;
    uint64_t startcycles;
    uint64_t startinsts;
    uint32_t exitJmpInst;
    JmpRepInfo *sysJmpinfos;
}RunningInfo;

void takeoverSyscall();
uint64_t loadelf(char * progname, char *ckptinfo);
void read_ckptinfo(char ckptinfo[]);

bool setJmp(uint64_t instaddr, uint64_t target);
void initMidJmpPlace();
void getRangeInfo(char filename[]);
void produceJmpInst(uint64_t npc);
void updateJmpInst(JmpRepInfo &info);


extern uint64_t takeOverAddr;
extern MemRangeInfo text_seg;

#define RunningInfoAddr 0x150000
#define StoreIntRegAddr "0x150028"
#define StoreFpRegAddr  "0x150128"
#define OldIntRegAddr   "0x150228"

#define TPoint1 0x100000
#define TPoint2 0x1FF000
#define MaxJALOffset 0xFFFF0

#define StartJump() asm volatile( \
    "jal x0, 0x100000  \n\t"   \
); 

#define Load_necessary(BaseAddr) asm volatile( \
    "li a0, " BaseAddr " \n\t"   \
    "ld sp,8*2(a0)  \n\t"   \
    "ld gp,8*3(a0)  \n\t"   \
    "ld tp,8*4(a0)  \n\t"   \
    "ld fp,8*8(a0)  \n\t"   \
); 

#define Context_Operation(Op, BaseAddr) asm volatile( \
    "li a0, " BaseAddr " \n\t"   \
    Op "0,8*0(a0)  \n\t"   \
    Op "1,8*1(a0)  \n\t"   \
    Op "2,8*2(a0)  \n\t"   \
    Op "3,8*3(a0)  \n\t"   \
    Op "4,8*4(a0)  \n\t"   \
    Op "5,8*5(a0)  \n\t"   \
    Op "6,8*6(a0)  \n\t"   \
    Op "7,8*7(a0)  \n\t"   \
    Op "8,8*8(a0)  \n\t"   \
    Op "9,8*9(a0)  \n\t"   \
    Op "11,8*11(a0)  \n\t"   \
    Op "12,8*12(a0)  \n\t"   \
    Op "13,8*13(a0)  \n\t"   \
    Op "14,8*14(a0)  \n\t"   \
    Op "15,8*15(a0)  \n\t"   \
    Op "16,8*16(a0)  \n\t"   \
    Op "17,8*17(a0)  \n\t"   \
    Op "18,8*18(a0)  \n\t"   \
    Op "19,8*19(a0)  \n\t"   \
    Op "20,8*20(a0)  \n\t"   \
    Op "21,8*21(a0)  \n\t"   \
    Op "22,8*22(a0)  \n\t"   \
    Op "23,8*23(a0)  \n\t"   \
    Op "24,8*24(a0)  \n\t"   \
    Op "25,8*25(a0)  \n\t"   \
    Op "26,8*26(a0)  \n\t"   \
    Op "27,8*27(a0)  \n\t"   \
    Op "28,8*28(a0)  \n\t"   \
    Op "29,8*29(a0)  \n\t"   \
    Op "30,8*30(a0)  \n\t"   \
    Op "31,8*31(a0)  \n\t"   \
    Op "10,8*10(a0)  \n\t"   \
);

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
