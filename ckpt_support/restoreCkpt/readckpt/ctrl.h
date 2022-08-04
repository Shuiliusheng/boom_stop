#ifndef   _PFC_ASM_  
#define   _PFC_ASM_  

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include"define.h"


uint64_t placefolds[1024];
uint64_t tempStackMem[4096];
uint64_t hpcounters[64];
uint64_t procTag=0x1234567;
uint64_t maxevent=10000, warmupinst=1000, eventsel=0, maxperiod=0;
uint64_t sampleHapTimes=0;
extern uint64_t __sampleFucAddr;


uint64_t startcycle = 0, endcycle = 0;
uint64_t startinst = 0, endinst = 0;

void sample_func();
void read_counters();
void init_start(uint64_t max_inst, uint64_t warmup_inst, uint64_t max_period);

void init_start(uint64_t max_inst, uint64_t warmup_inst, uint64_t max_period)
{ 
    printf("warmup: %d, maxinst: %d, period: %d\n", warmup_inst, max_inst, max_period);
    startcycle = read_csr_cycle();
    startinst = read_csr_instret();

    Save_Basic_Regs();
    tempStackMem[32] = (uint64_t) &tempStackMem[2048]; //set sp
    tempStackMem[33] = (uint64_t) &tempStackMem[2560]; //set s0 = sp + 512*8
    printf("sample function addr: 0x%lx, sp: 0x%lx, s0: 0x%lx\n", &__sampleFucAddr, tempStackMem[32], tempStackMem[33]); 

    maxevent = 10000000, warmupinst = 10000000, eventsel=0, maxperiod = max_period;
    SetCounterLevel("0");
	RESET_COUNTER;
    SetSampleBaseInfo(procTag, &__sampleFucAddr);
    SetSampleCtrlReg(maxevent, warmupinst, eventsel);
    SetPfcEnable(1);
}


void sample_func()
{
    asm volatile("__sampleFucAddr: ");
    Save_ALLIntRegs();
    Load_Basic_Regs();

    uint64_t exitpc=0;
    GetExitPC(exitpc);
    SetTempReg(exitpc, 0);
    sampleHapTimes++;

    endcycle = read_csr_cycle();
    endinst = read_csr_instret();
    char str[256];
    sprintf(str, "{\"type\": \"max_inst\", \"times\": %d, \"cycles\": %ld, \"inst\": %ld}\n", sampleHapTimes, endcycle - startcycle, endinst - startinst);
    write(1, str, strlen(str));  

    read_counters();
    startcycle = endcycle;
    startinst = endinst;
    sprintf(str, "exitpc: 0x%lx\n", exitpc);
    write(1, str, strlen(str)); 
    for(int i=0;i<32;i++) {
        sprintf(str, "reg %d: 0x%lx\n", i, tempStackMem[i]);
        write(1, str, strlen(str)); 
    }

    // if(sampleHapTimes >= maxperiod) {
    //     exit(0);
    // }

    warmupinst = 0;
    RESET_COUNTER;
    SetPfcEnable(1);
    SetSampleBaseInfo(procTag, &__sampleFucAddr);
    SetSampleCtrlReg(maxevent, warmupinst, eventsel);
    Load_ALLIntRegs();
    JmpTempReg(0);
}



void read_counters()
{
	ReadCounter16(&hpcounters[0], 0);
	ReadCounter16(&hpcounters[16], 16);
	ReadCounter16(&hpcounters[32], 32);
	ReadCounter16(&hpcounters[48], 48);

    char str[256];
	for(int n=50;n<53;n++){
		sprintf(str, "{\"type\": \"event %2d\", \"value\": %llu}\n", n, hpcounters[n]);
        write(1, str, strlen(str));
    }
}

#endif 
