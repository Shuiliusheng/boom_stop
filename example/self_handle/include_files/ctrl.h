#ifndef   _PFC_CTRL_H_  
#define   _PFC_CTRL_H_

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include <fcntl.h>
#include <stdlib.h>

#include"define.h"

bool read_params(const char filename[]);
void sample_func(uint64_t sampletimes, uint64_t exitpc);
void based_sample_func();
__attribute((constructor)) void initStart();

uint64_t procTag=0x1234567;
uint64_t sampleHapTimes = 0;
uint64_t placefolds[1024];
uint64_t tempStackMem[4096];
extern uint64_t __sampleFucAddr;
uint64_t maxevent = 100, eventsel = 0, warmupinst = 0, maxperiod = 0;

__attribute((constructor)) void initStart()
{
    eventsel = 0, maxevent = 20000, warmupinst = 0, maxperiod=1000;
    printf("eventsel: %d, maxevent: %d, warmup: %d, maxperiod: %d\n", eventsel, maxevent, warmupinst, maxperiod);
    
    Save_Basic_Regs();
    tempStackMem[32] = (uint64_t) &tempStackMem[2048]; //set sp
    tempStackMem[33] = (uint64_t) &tempStackMem[2560]; //set s0 = sp + 512*8
    printf("sample function addr: 0x%lx, sp: 0x%lx, s0: 0x%lx\n", &__sampleFucAddr, tempStackMem[32], tempStackMem[33]); 

    SetCounterLevel("0");
    SetPfcEnable(1);
    SetSampleBaseInfo(procTag, &__sampleFucAddr);
    SetSampleCtrlReg(maxevent, warmupinst, eventsel);
    RESET_COUNTER;
}


void based_sample_func()
{
    asm volatile("__sampleFucAddr: ");
    Save_ALLIntRegs();
    Load_Basic_Regs();

    uint64_t exitpc = 0;
    GetExitPC(exitpc);
    SetTempReg(exitpc, 0);
    sample_func(sampleHapTimes, exitpc);

    sampleHapTimes++;
    warmupinst = 0;
    SetPfcEnable(1);
    SetSampleBaseInfo(procTag, &__sampleFucAddr);
    SetSampleCtrlReg(maxevent, warmupinst, eventsel);
    RESET_COUNTER;

    Load_ALLIntRegs();
    JmpTempReg(0);
}


uint64_t startcycle, endcycle, startinst, endinst;
uint64_t hpcounters[64];
void sample_func(uint64_t sampletimes, uint64_t exitpc)
{
    char str[300];
    endcycle = read_csr_cycle();
    endinst = read_csr_instret();

    sprintf(str, "{\"type\": \"max_inst\", \"times\": %d, \"cycles\": %ld, \"inst\": %ld}\n", sampletimes, endcycle - startcycle, endinst - startinst);
    write(1, str, strlen(str));

    startcycle = endcycle;
    startinst = endinst;

    ReadCounter16(&hpcounters[0], 0);
    ReadCounter16(&hpcounters[16], 16);
    ReadCounter16(&hpcounters[32], 32);
    ReadCounter16(&hpcounters[48], 48);
	for(int n=0;n<2;n++){
		sprintf(str, "{\"type\": \"event %2d\", \"value\": %llu}\n", n, hpcounters[n]);
        write(1, str, strlen(str));
    }
}



#endif 
