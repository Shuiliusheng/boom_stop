#ifndef   _PFC_ASM_  
#define   _PFC_ASM_  



#define RESET_COUNTER asm volatile( \
	" andi x0, t0, 64      \n\t" \
);

unsigned long long read_counter(int n){
	unsigned long long temp=0;
	switch(n){
		case 0: asm volatile( " andi x0,t0,32    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 1: asm volatile( " andi x0,t0,33    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 2: asm volatile( " andi x0,t0,34    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 3: asm volatile( " andi x0,t0,35    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 4: asm volatile( " andi x0,t0,36    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 5: asm volatile( " andi x0,t0,37    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 6: asm volatile( " andi x0,t0,38    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 7: asm volatile( " andi x0,t0,39    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 8: asm volatile( " andi x0,t0,40    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 9: asm volatile( " andi x0,t0,41    \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 10: asm volatile( " andi x0,t0,42   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 11: asm volatile( " andi x0,t0,43   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 12: asm volatile( " andi x0,t0,44   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 13: asm volatile( " andi x0,t0,45   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 14: asm volatile( " andi x0,t0,46   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 15: asm volatile( " andi x0,t0,47   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
                                                                        
        case 16: asm volatile( " andi x0,t0,48   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 17: asm volatile( " andi x0,t0,49   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 18: asm volatile( " andi x0,t0,50   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 19: asm volatile( " andi x0,t0,51   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 20: asm volatile( " andi x0,t0,52   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 21: asm volatile( " andi x0,t0,53   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 22: asm volatile( " andi x0,t0,54   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 23: asm volatile( " andi x0,t0,55   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 24: asm volatile( " andi x0,t0,56   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 25: asm volatile( " andi x0,t0,57   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 26: asm volatile( " andi x0,t0,58   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 27: asm volatile( " andi x0,t0,59   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 28: asm volatile( " andi x0,t0,60   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 29: asm volatile( " andi x0,t0,61   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 30: asm volatile( " andi x0,t0,62   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
        case 31: asm volatile( " andi x0,t0,63   \n\t" " add %[o1],t0,0   \n\t" :[o1]"=r"(temp) : ); break;
		default: break;
	}
	return temp;
}

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


unsigned long long start_values[32] = {0};

__attribute((constructor)) void start_record(){

    SetCounterLevel("3");

    unsigned int tag = 0x1234567, t1 = 0;
    SetCtrlReg(tag, t1, t1, t1);

	printf("start read performance counters\n");
    RESET_COUNTER;
	for(int n=0;n<32;n++){
		start_values[n]=read_counter(n);
	}
}

__attribute((destructor)) void exit_record(){
	unsigned long long exit_values[32] = {0};
	int n=0;
	for(n=0;n<32;n++){
		exit_values[n]=read_counter(n);
	}

	for(n=0;n<32;n++){
		printf("event %2d: before: %10llu, after: %10llu, before-after: %10llu\n", n, start_values[n], exit_values[n], exit_values[n]-start_values[n]);
	}
}

#endif 