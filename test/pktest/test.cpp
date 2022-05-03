#include "test.h"
#include<unistd.h>
#include<string.h>
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
unsigned long long exitFucAddr=0x10626;
unsigned long long maxinst=10000;

char str[300];

void display(int fd, char *buf, int size);

void exit_fuc()
{
    Save_int_regs();
    Load_necessary();

    GetNPC(npc);
    SetNPC(npc);
    exittime++;

    endcycle = __csrr_cycle();
    endinst = __csrr_instret();
    sprintf(str, "exit %d, cycles: %ld, inst: %ld\n", exittime, endcycle - startcycle, endinst - startinst);
    write(1,str,strlen(str));
    // exit(1);

    startcycle = endcycle;
    startinst = endinst;

    SetCtrlReg(tag, exitFucAddr, maxinst);
    Load_regs();
    URet();
}

//直接使用printf作为exit_fuc中的输出函数，会影响原本printf中的执行，从而导致出错，因此单独增加一个display的函数
void display(int fd, char *buf, int size)
{
    asm volatile( 
        "li	a7,64 \n\t"   
        "ecall  \n\t"   
    ); 
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
    
//    printf("intput max insts: ");
  //  scanf("%d", &maxinst);

    startcycle = __csrr_cycle();
    startinst = __csrr_instret();
    SetCtrlReg(tag, exitFucAddr, maxinst);



    printf("hello world\n");
    for(int i=0;i<10000;i++){
        printf("hello world: %d\n", i);
    }
    printf("exittime: %d\n", exittime);
    return 0;
}
