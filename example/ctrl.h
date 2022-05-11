#ifndef   _PFC_ASM_  
#define   _PFC_ASM_  

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"define.h"

unsigned long long exit_counters[64];

unsigned long long intregs[32];
unsigned long long necessaryRegs[1000];

unsigned long long npc=0, exittime=0;
unsigned long long procTag=0x1234567;
unsigned long long exitFucAddr=0x10626;
unsigned long long maxinst=100000, warmupinst=1000;
char str_temp[256];

unsigned long long startcycle = 0, endcycle = 0;
unsigned long long startinst = 0, endinst = 0;

void exit_record();

void exit_fuc()
{
    Save_ALLIntRegs();
    Load_Basic_Regs();

    GetNPC(npc);
    SetNPC(npc);
    exittime++;

    endcycle = read_csr_cycle();
    endinst = read_csr_instret();
    sprintf(str_temp, "exit maxinst, cycles: %ld, inst: %ld\n", endcycle - startcycle, endinst - startinst);
    write(1, str_temp, strlen(str_temp));
    exit_record();
    // exit(1);

    startcycle = endcycle;
    startinst = endinst;
    warmupinst = 0;
    SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst);
    Load_ALLIntRegs();
    URet();
}

__attribute((constructor)) void init_start()
{

    unsigned long long t1 = (unsigned long long)&intregs[0];
    unsigned long long t2 = (unsigned long long)&necessaryRegs[0];
    unsigned long long t3 = 0;
    necessaryRegs[0]=(unsigned long long)&necessaryRegs[400];
    SetTempRegs(t1, t2, t3);
    Save_Basic_Regs();
    SetCounterLevel("0");

    // printf("intput max insts: ");
    // scanf("%d", &maxinst);

	RESET_COUNTER;
    SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst);
}

void exit_record()
{
	
	ReadCounter16(&exit_counters[0], 0);
	ReadCounter16(&exit_counters[16], 16);
	ReadCounter16(&exit_counters[32], 32);
	ReadCounter16(&exit_counters[48], 48);

	for(int n=0;n<64;n++){
		sprintf(str_temp, "event %2d: exit_counters: %llu\n", n, exit_counters[n]);
        write(1, str_temp, strlen(str_temp));
    }

    RESET_COUNTER;
}

#endif 