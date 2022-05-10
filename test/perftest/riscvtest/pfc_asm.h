#ifndef   _PFC_ASM_  
#define   _PFC_ASM_  



#define RESET_COUNTER asm volatile(" andi x0, t0, 1024 \n\t" );

#define GetCounter(basereg, dstreg, start, n)    \
              "andi x0, " dstreg ", 512+" #start "+" #n " \n\t" \
              "sd " dstreg ", " #n "*8(" basereg ") \n\t"

#define ReadCounter8(base, start) asm volatile( \
    "mv t1, %[addr]  # set base addr \n\t"  \
    GetCounter("t1", "t0", start, 0)  \
    GetCounter("t1", "t0", start, 1)  \
    GetCounter("t1", "t0", start, 2)  \
    GetCounter("t1", "t0", start, 3)  \
    GetCounter("t1", "t0", start, 4)  \
    GetCounter("t1", "t0", start, 5)  \
    GetCounter("t1", "t0", start, 6)  \
    GetCounter("t1", "t0", start, 7)  \
    : \
    :[addr]"r"(base) \
); 

#define ReadCounter16(base, start) \
    ReadCounter8(base, start) \
    ReadCounter8(base+8, start+8) 


//设置计数器需要统计的级别
#define SetCounterLevel(priv) asm volatile( \
    "li t0, " priv "  # priv: user/super/machine \n\t"  \
    "addi x0, t0, 6 \n\t"  \
); 


//设置硬件一些临时寄存器的值，以此控制程序执行
//processTag：设置为tag的值，仅当为0x1234567时才进行各种操作，包括退出和事件计数
//exitFuncAddr：设置为exitFucAddr，仅当不为0并且procMaxInsts也不为0时有效。控制退出事件时的跳转目标地址
//procMaxInsts：设置为maxinst，控制当进程的用户态指令数达到maxinsts时，发生退出情况
//startInsts：设置为startinst，控制当进程的用户态指令数达到startinst时，重置计数器
#define SetCtrlReg(tag, exitFucAddr, maxinst, startinst) asm volatile( \
    "mv t0, %[rtemp1]  # tag \n\t"  \
    "addi x0, t0, 1 \n\t"  \
    "mv t0, %[rtemp2]  # exit \n\t"  \
    "addi x0, t0, 2 \n\t"  \
    "mv t0, %[rtemp3]  # maxinst \n\t"  \
    "addi x0, t0, 3 \n\t"  \
    "mv t0, %[rtemp4]  # startinst \n\t"  \
    "addi x0, t0, 10 \n\t"  \
    : \
    :[rtemp1]"r"(tag), [rtemp2]"r"(exitFucAddr), [rtemp3]"r"(maxinst), [rtemp4]"r"(startinst)\
); 


unsigned long long start_values[64] = {0};

__attribute((constructor)) void start_record(){

    SetCounterLevel("3");

    unsigned int tag = 0x1234567, t1 = 0;
    SetCtrlReg(tag, t1, t1, t1);

	printf("start read performance counters\n");
    RESET_COUNTER;
	ReadCounter16(&start_values[0], 0);
	ReadCounter16(&start_values[16], 16);
	ReadCounter16(&start_values[32], 32);
}

void exit_record(){
	unsigned long long exit_values[64] = {0};
	int n=0;
	ReadCounter16(&exit_values[0], 0);
	ReadCounter16(&exit_values[16], 16);
	ReadCounter16(&exit_values[32], 32);

	for(n=0;n<48;n++){
		printf("event %2d: before: %lu, after: %lu, before-after: %lu\n", n, start_values[n], exit_values[n], exit_values[n]-start_values[n]);
	}
}

#endif 