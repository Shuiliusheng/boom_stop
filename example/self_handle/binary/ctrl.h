#ifndef   _PFC_CTRL_H_  
#define   _PFC_CTRL_H_

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
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
extern uint64_t maxevent, eventsel, warmupinst, maxperiod;

__attribute((constructor)) void initStart()
{
    eventsel = 0, maxevent = 20000, warmupinst = 0, maxperiod=1000;
    read_params("samplectrl.txt");

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


#endif 
