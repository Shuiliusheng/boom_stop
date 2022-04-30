#include<stdio.h>
#include<stdlib.h>

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

#define SetSTemReg(src1, src2, src3) asm volatile( \
    "mv t0, %[rtemp1]  # tag \n\t"  \
    "addi x0, t0, 1 \n\t"  \
    "mv t0, %[rtemp2]  # exit \n\t"  \
    "addi x0, t0, 2 \n\t"  \
    "mv t0, %[rtemp3]  # maxinst \n\t"  \
    "addi x0, t0, 3 \n\t"  \
    : \
    :[rtemp1]"r"(src1), [rtemp2]"r"(src2), [rtemp3]"r"(src3)  \
); 

unsigned long long startcycle = 0, endcycle = 0;
unsigned long long startinst = 0, endinst = 0;

void exit_fuc()
{
    endcycle = __csrr_cycle();
    endinst = __csrr_instret();
    printf("exit function! (%ld %ld)  (%ld %ld)\n", startcycle, startinst, endcycle, endinst);
}

int main()
{
    unsigned long long tag=0x1234567;
    unsigned long long exitFucAddr=0x80002000;
    unsigned long long maxinst=16000;
    startcycle = __csrr_cycle();
    startinst = __csrr_instret();

    SetSTemReg(tag, exitFucAddr, maxinst);

    printf("hello world\n");
    for(int i=0;i<1000;i++){
        printf("hello world: %d\n", i);
    }
    return 0;
}
