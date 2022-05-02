#include "test.h"
#define GetNPC(npc) asm volatile( \
    "addi x0, t0, 1027 \n\t"  \
    "mv %[npc], t0  # uretaddr \n\t"  \
    : [npc]"=r"(npc)\
    :  \
); 

#define SetNPC(npc) asm volatile( \
    "mv t0, %[npc]  # uretaddr \n\t"  \
    "addi x0, t0, 5 \n\t"  \
    : \
    :[npc]"r"(npc)  \
); 
#define URet() asm volatile( \
    "addi x0, x0, 128  # uret \n\t"  \
); 


unsigned long long intregs[32];
unsigned long long necessaryRegs[1000];

unsigned long long npc=0;
unsigned long long exittime=0;
unsigned long long tag=0x1234567;
unsigned long long exitFucAddr=0x80002008;
unsigned long long maxinst=1000;

char str[300];

void exit_fuc()
{
    Save_int_regs();
    Load_necessary();

    GetNPC(npc);
    SetNPC(npc);
    exittime++;

    endcycle = __csrr_cycle();
    endinst = __csrr_instret();
    // sprintf(str, "exit function! %ld %ld\n", endcycle, endinst);
    // printstr(str);
    printf("exit function! %ld %ld\n", endcycle, endinst);
    printf("exit function! %ld %ld\n", endcycle, endinst);
    printf("exit function! %ld %ld\n", endcycle, endinst);
    printf("exit function! %ld %ld\n", endcycle, endinst);
    printf("exit function! %ld %ld\n", endcycle, endinst);

    SetCtrlReg(tag, exitFucAddr, maxinst);
    Load_int_regs();
    URet();
}


int main()
{
    unsigned long long t1 = (unsigned long long)&intregs[0];
    unsigned long long t2 = (unsigned long long)&necessaryRegs[0];
    unsigned long long t3 = 0;
    necessaryRegs[0]=(unsigned long long)&necessaryRegs[400];
    printf("t1: 0x%lx, t2: 0x%lx\n", t1, t2);
    SetTempReg(t1, t2, t3);
    Save_necessary();

    startcycle = __csrr_cycle();
    startinst = __csrr_instret();
    SetCtrlReg(tag, exitFucAddr, maxinst);

    // printf("hello world\n");
    for(int i=0;i<10000;i++){
        // printf("hello world: %d\n", i);
        t1 = t2 + t3;
        t2 = t2 + t3;
        t3 = t2 + t3;
    }
    intregs[0]=t1;
    // printf("exittime: %d\n", exittime);
    return 0;
}
