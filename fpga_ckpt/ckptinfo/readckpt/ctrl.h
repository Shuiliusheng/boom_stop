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
unsigned long long exitFucAddr=0x10f68;
unsigned long long maxinst=1000000, warmupinst=0;
char str_temp[300];

unsigned long long startcycle = 0, endcycle = 0;
unsigned long long startinst = 0, endinst = 0;

void exit_record();

void exit_fuc()
{
    //exitFucAddr指向Save_int_regs的第一条指令
    //将当前进程的寄存器信息保存下来，类似于正常的异常处理程序的入口
    Save_ALLIntRegs();
    //和Save_necessary对应，sp会被设置成新的值（necessRegs[400]）
    Load_Basic_Regs();

    //获取进程进入exit_fuc之前程序将要执行的下一条指令PC，同时将其设置在硬件临时寄存器中，用于之后跳转回去
    GetNPC(npc);
    SetNPC(npc);
    exittime++;

    //用于获取当前计数器的值，并进行输出（以json的格式输出，便于之后处理）
    exit_record();
    // exit(1);

    startcycle = endcycle;
    startinst = endinst;
    warmupinst = 0;
    //重新设置下一次触发退出的条件
    SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst);
    //将Save_int_regs保存的内容恢复回去
    Load_ALLIntRegs();
    //根据SetNPC的内容进行跳转
    URet();
}

void init_start()
{
    //----------------------------------------------------
    //用于保存一些必要的寄存器值， 同时将necessRegs[400]作为新的sp，用于临时使用
    unsigned long long t1 = (unsigned long long)&intregs[0];
    unsigned long long t2 = (unsigned long long)&necessaryRegs[0];
    unsigned long long t3 = 0;
    necessaryRegs[0]=(unsigned long long)&necessaryRegs[400];
    SetTempRegs(t1, t2, t3);
    Save_Basic_Regs();

    //-------------------------------------------------------
    //用于设置计数器需要统计哪些级别的时间，0表示仅统计用户态，1表示统计user+super，3表示user+super+machine
    SetCounterLevel("0");

    //在运行时输入 maxinst
    // printf("intput max insts: ");
    // scanf("%d", &maxinst);

    //-------------------------------------------------------
    //用于将所有计数器重设为0
	RESET_COUNTER;

    //-------------------------------------------------------
    SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst);
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

    RESET_COUNTER;
}

#endif 