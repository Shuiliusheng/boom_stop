#include<stdio.h>
#include<stdlib.h>

unsigned long long startcycle = 0, endcycle = 0;
unsigned long long startinst = 0, endinst = 0;


#define SetCtrlReg(tag, exitFucAddr, maxinst) asm volatile( \
    "li t0, 0  # priv: machine \n\t"  \
    "addi x0, t0, 6 \n\t"  \
    "mv t0, %[rtemp1]  # tag \n\t"  \
    "addi x0, t0, 1 \n\t"  \
    "mv t0, %[rtemp2]  # exit \n\t"  \
    "addi x0, t0, 2 \n\t"  \
    "mv t0, %[rtemp3]  # maxinst \n\t"  \
    "addi x0, t0, 3 \n\t"  \
    : \
    :[rtemp1]"r"(tag), [rtemp2]"r"(exitFucAddr), [rtemp3]"r"(maxinst)  \
); 


#define SetTempReg(t1, t2, t3) asm volatile( \
    "mv t0, %[rtemp1]  # tag \n\t"  \
    "addi x0, t0, 7 \n\t"  \
    "mv t0, %[rtemp2]  # exit \n\t"  \
    "addi x0, t0, 8 \n\t"  \
    "mv t0, %[rtemp3]  # maxinst \n\t"  \
    "addi x0, t0, 9 \n\t"  \
    : \
    :[rtemp1]"r"(t1), [rtemp2]"r"(t2), [rtemp3]"r"(t3)  \
);


#define Load_necessary() asm volatile( \
    "addi x0, t0, 1029 \n\t"   \
    "ld sp,8*0(t0)  \n\t"   \
    "ld gp,8*1(t0)  \n\t"   \
    "ld tp,8*2(t0)  \n\t"   \
    "ld fp,8*3(t0)  \n\t"   \
    "ld ra,8*4(t0)  \n\t"   \
); 

#define Save_necessary() asm volatile( \
    "addi x0, t0, 1029 \n\t"   \
    "sd gp,8*1(t0)  \n\t"   \
    "sd tp,8*2(t0)  \n\t"   \
    "sd fp,8*3(t0)  \n\t"   \
    "sd ra,8*4(t0)  \n\t"   \
); 

#define Save_int_regs() asm volatile( \
    "addi x0, a0, 4  # write a0 to uscratch \n\t"   \
    "addi x0, a0, 1028 # read temp1 to a0 \n\t"   \
    "sd x1,8*1(a0)  \n\t"   \
    "sd x2,8*2(a0)  \n\t"   \
    "sd x3,8*3(a0)  \n\t"   \
    "sd x4,8*4(a0)  \n\t"   \
    "sd x5,8*5(a0)  \n\t"   \
    "sd x6,8*6(a0)  \n\t"   \
    "sd x7,8*7(a0)  \n\t"   \
    "sd x8,8*8(a0)  \n\t"   \
    "sd x9,8*9(a0)  \n\t"   \
    "sd x10,8*10(a0)  \n\t"   \
    "sd x11,8*11(a0)  \n\t"   \
    "sd x12,8*12(a0)  \n\t"   \
    "sd x13,8*13(a0)  \n\t"   \
    "sd x14,8*14(a0)  \n\t"   \
    "sd x15,8*15(a0)  \n\t"   \
    "sd x16,8*16(a0)  \n\t"   \
    "sd x17,8*17(a0)  \n\t"   \
    "sd x18,8*18(a0)  \n\t"   \
    "sd x19,8*19(a0)  \n\t"   \
    "sd x20,8*20(a0)  \n\t"   \
    "sd x21,8*21(a0)  \n\t"   \
    "sd x22,8*22(a0)  \n\t"   \
    "sd x23,8*23(a0)  \n\t"   \
    "sd x24,8*24(a0)  \n\t"   \
    "sd x25,8*25(a0)  \n\t"   \
    "sd x26,8*26(a0)  \n\t"   \
    "sd x27,8*27(a0)  \n\t"   \
    "sd x28,8*28(a0)  \n\t"   \
    "sd x29,8*29(a0)  \n\t"   \
    "sd x30,8*30(a0)  \n\t"   \
    "sd x31,8*31(a0)  \n\t"   \
);  

#define Load_regs() asm volatile( \
    "addi x0, a0, 1028 # read temp1 to a0 \n\t"   \
    "ld x1,8*1(a0)  \n\t"   \
    "ld x2,8*2(a0)  \n\t"   \
    "ld x3,8*3(a0)  \n\t"   \
    "ld x4,8*4(a0)  \n\t"   \
    "ld x5,8*5(a0)  \n\t"   \
    "ld x6,8*6(a0)  \n\t"   \
    "ld x7,8*7(a0)  \n\t"   \
    "ld x8,8*8(a0)  \n\t"   \
    "ld x9,8*9(a0)  \n\t"   \
    "ld x11,8*11(a0)  \n\t"   \
    "ld x12,8*12(a0)  \n\t"   \
    "ld x13,8*13(a0)  \n\t"   \
    "ld x14,8*14(a0)  \n\t"   \
    "ld x15,8*15(a0)  \n\t"   \
    "ld x16,8*16(a0)  \n\t"   \
    "ld x17,8*17(a0)  \n\t"   \
    "ld x18,8*18(a0)  \n\t"   \
    "ld x19,8*19(a0)  \n\t"   \
    "ld x20,8*20(a0)  \n\t"   \
    "ld x21,8*21(a0)  \n\t"   \
    "ld x22,8*22(a0)  \n\t"   \
    "ld x23,8*23(a0)  \n\t"   \
    "ld x24,8*24(a0)  \n\t"   \
    "ld x25,8*25(a0)  \n\t"   \
    "ld x26,8*26(a0)  \n\t"   \
    "ld x27,8*27(a0)  \n\t"   \
    "ld x28,8*28(a0)  \n\t"   \
    "ld x29,8*29(a0)  \n\t"   \
    "ld x30,8*30(a0)  \n\t"   \
    "ld x31,8*31(a0)  \n\t"   \
    "addi x0, a0, 1026  \n\t"   \
); 


#define DEFINE_CSRR(s)                     \
    static inline unsigned long long __csrr_##s()    \
    {                                      \
        unsigned long long value;                    \
        __asm__ volatile("csrr    %0, " #s \
                         : "=r"(value)     \
                         :);               \
        return value;                      \
    }

DEFINE_CSRR(cycle)
DEFINE_CSRR(instret)


