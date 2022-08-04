#include "define.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

uint64_t procTag=0x1234567;
uint64_t maxevent=0, warmupinst=0, eventsel=0;
uint64_t sampleFuncAddr = 0;
uint64_t hpcounters[64];
uint64_t tempStackMem[4096];

int main()
{
    SetSampleBaseInfo(procTag, sampleFuncAddr);
    SetSampleCtrlReg(maxevent, warmupinst, eventsel);
    SetCounterLevel("3");
    SetPfcEnable(1);

    uint64_t t1 = 1, t2 = 2, t3 = 3;
    for(int i=0;i<10000;i++){
        t1 = t2 + t3;
        t2 = t2 + t3;
        t3 = t2 + t3;
        tempStackMem[i%4096] = t1 + t2 + t3;
    }

    ReadCounter16(&hpcounters[0], 0);
	ReadCounter16(&hpcounters[16], 16);
	ReadCounter16(&hpcounters[32], 32);
	ReadCounter16(&hpcounters[48], 48);

	for(int n=0;n<64;n++){
		printf("{\"type\": \"event %2d\", \"value\": %llu}\n", n, hpcounters[n]);
    }

    return 0;
}
