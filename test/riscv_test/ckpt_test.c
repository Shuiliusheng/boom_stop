#include<stdio.h>

#define SetCkptTempReg(t0, t1, t2, t3) asm volatile( \
    "mv t0, %[rtemp0]  # t0 \n\t"  \
    "ori x0, t0, 8 \n\t"  \
    "mv t0, %[rtemp1]  # t1 \n\t"  \
    "ori x0, t0, 9 \n\t"  \
    "mv t0, %[rtemp2]  # t2 \n\t"  \
    "ori x0, t0, 10 \n\t"  \
    "mv t0, %[rtemp3]  # t3 \n\t"  \
    "ori x0, t0, 11 \n\t"  \
    : \
    :[rtemp0]"r"(t0), [rtemp1]"r"(t1), [rtemp2]"r"(t2), [rtemp3]"r"(t3)  \
);

#define GetCkptTempReg(t0, t1, t2, t3) asm volatile( \
    "ori x0, t0, 4 \n\t"  \
    "mv %[wt0], t0  # t0 \n\t"  \
    "ori x0, t0, 5 \n\t"  \
    "mv %[wt1], t0  # t0 \n\t"  \
    "ori x0, t0, 6 \n\t"  \
    "mv %[wt2], t0  # t0 \n\t"  \
    "ori x0, t0, 7 \n\t"  \
    "mv %[wt3], t0  # t0 \n\t"  \
    :[wt0]"=r"(t0), [wt1]"=r"(t1),[wt2]"=r"(t2),[wt3]"=r"(t3) \
    : \
);

#define JmpTempToAddr(t2) asm volatile( \
    "mv t0, %[rtemp2]  # t2 \n\t"  \
    "ori x0, t0, 10  # rtemp2 \n\t"  \
    "ori x0, x0, 14 # jmp rtemp2 \n\t"  \
    : \
    :[rtemp2]"r"(t2) \
);


int main()
{
    unsigned long long t1 = 0, t2 = 0;
    unsigned long long t3 = 0, t4 = 0;

    t1 = 1, t2 = 2, t3 = 3, t4 = 4;
    SetCkptTempReg(t1, t2, t3, t4);
    printf("%d, %d, %d, %d\n", t1, t2, t3, t4);

    t1 = 11, t2 = 21, t3 = 31, t4 = 41;
    GetCkptTempReg(t1, t2, t3, t4);
    printf("%d, %d, %d, %d\n", t1, t2, t3, t4);

    t3 = 0x80002766;
    JmpTempToAddr(t3);

    printf("set jmp over\n");
    printf("jmp target is here\n");

    return 0;
}
