#ifndef   _PFC_ASM_  
#define   _PFC_ASM_  

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"define.h"

unsigned long long exit_values[32];

unsigned long long intregs[32];
unsigned long long necessaryRegs[1000];

unsigned long long npc=0, exittime=0;
unsigned long long procTag=0x1234567;
unsigned long long exitFucAddr=0x10828;
unsigned long long maxinst=10000, warmupinst=1000;
char str_temp[300];

unsigned long long startcycle = 0, endcycle = 0;
unsigned long long startinst = 0, endinst = 0;

void exit_record();

void exit_fuc()
{
    //exitFucAddr指向Save_int_regs的第一条指令
    //将当前进程的寄存器信息保存下来，类似于正常的异常处理程序的入口
    Save_int_regs();
    //和Save_necessary对应，sp会被设置成新的值（necessRegs[400]）
    Load_necessary();

    //获取进程进入exit_fuc之前程序将要执行的下一条指令PC，同时将其设置在硬件临时寄存器中，用于之后跳转回去
    GetNPC(npc);
    SetNPC(npc);
    exittime++;

    endcycle = __csrr_cycle();
    endinst = __csrr_instret();
    sprintf(str_temp, "\"type\": \"max_inst\", \"cycles\": %ld, \"inst\": %ld\n", endcycle - startcycle, endinst - startinst);
    //write函数：使用printf函数可能会导致和源程序的printf函数冲突，可能引发临界区资源的冲突而卡死
    write(1, str_temp, strlen(str_temp));
    //用于获取当前计数器的值，并进行输出（以json的格式输出，便于之后处理）
    exit_record();
    // exit(1);

    startcycle = endcycle;
    startinst = endinst;
    warmupinst = 0;
    //重新设置下一次触发退出的条件
    SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst);
    //将Save_int_regs保存的内容恢复回去
    Load_int_regs();
    //根据SetNPC的内容进行跳转
    URet();
}

__attribute((constructor)) void init_start()
{
    //----------------------------------------------------
    //用于保存一些必要的寄存器值， 同时将necessRegs[400]作为新的sp，用于临时使用
    unsigned long long t1 = (unsigned long long)&intregs[0];
    unsigned long long t2 = (unsigned long long)&necessaryRegs[0];
    unsigned long long t3 = 0;
    necessaryRegs[0]=(unsigned long long)&necessaryRegs[400];
    SetTempReg(t1, t2, t3);
    Save_necessary();

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
    //procTag: 设置当前进程的TAG，只有当该TAG为0x1234567时，才能够统计事件，自动退出等
    //exitFucAddr: 当进程达到最大执行执行数时，跳转的入口地址，通常指向exit_fuc的入口。当为0时，不发生任何跳转；
    //maxinst：设置从当前开始，程序能够执行的最大指令数（用户态指令）。如果为0，则无任何影响；
    //warmupinst：设置从当前开始，执行多少条指令之后，cpu自动reset所有计数器的值
    SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst);
}

void exit_record()
{
	int n=0;
	for(n=0;n<8;n++){
		exit_values[n]=read_counter(n);
	}

	for(n=0;n<8;n++){
		sprintf(str_temp, "{\"type\": \"event %2d\", \"value\": %10llu\n", n, exit_values[n]);
        write(1, str_temp, strlen(str_temp));
    }

    RESET_COUNTER;
}

#endif 