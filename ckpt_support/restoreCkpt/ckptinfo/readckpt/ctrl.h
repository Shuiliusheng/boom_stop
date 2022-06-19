#ifndef   _PFC_ASM_  
#define   _PFC_ASM_  

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"define.h"

unsigned long long exit_counters[64];

unsigned long long intregs[32];
unsigned long long necessaryRegs[1000];

unsigned long long npc=0;
unsigned long long procTag=0x1234567;
unsigned long long exitFucAddr=0x10f68;
unsigned long long maxinst=1000000, warmupinst=0;
char str_temp[300];

unsigned long long startcycle = 0, endcycle = 0;
unsigned long long startinst = 0, endinst = 0;
extern unsigned long long __exitFucAddr;

void exit_record();
void exit_fuc()
{
    asm volatile("__exitFucAddr: ");
    Save_ALLIntRegs();
    Load_Basic_Regs();

    GetNPC(npc);
    SetNPC(npc);

    endcycle = read_csr_cycle();
    endinst = read_csr_instret();
    sprintf(str_temp, "total cycles: %ld,  total inst: %ld\n", endcycle - startcycle, endinst - startinst);
    write(1, str_temp, strlen(str_temp));

    exit_record();
    // exit(0);

    // warmupinst = 30000;
    // SetCtrlReg(procTag, &__exitFucAddr, maxinst, warmupinst);
    Load_ALLIntRegs();
    URet();
}

void init_start(unsigned long long max_inst, unsigned long long warmup_inst)
{
    //----------------------------------------------------
    //用于保存一些必要的寄存器值， 同时将necessRegs[400]作为新的sp，用于临时使用
    unsigned long long t1 = (unsigned long long)&intregs[0];
    unsigned long long t2 = (unsigned long long)&necessaryRegs[0];
    unsigned long long t3 = 0;
    necessaryRegs[0]=(unsigned long long)&necessaryRegs[400];
    SetTempRegs(t1, t2, t3);
    Save_Basic_Regs();
    SetCounterLevel("0");
    startcycle = read_csr_cycle();
    startinst = read_csr_instret();
	RESET_COUNTER;
    SetCtrlReg(procTag, &__exitFucAddr, max_inst, warmup_inst);
}

void exit_record()
{
	ReadCounter16(&exit_counters[0], 0);
	ReadCounter16(&exit_counters[16], 16);
	ReadCounter16(&exit_counters[32], 32);

	for(int n=0;n<48;n++){
		sprintf(str_temp, "{\"type\": \"event %2d\", \"value\": %llu}\n", n, exit_counters[n]);
        write(1, str_temp, strlen(str_temp));
    }
}

#endif 