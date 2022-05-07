#ifndef   _DEFINE_H_  
#define   _DEFINE_H_


//-------------------------------------------------------------------
//获取发生exit实现时下一条指令的pc
#define GetNPC(npc) asm volatile( \
    "addi x0, t0, 1027 \n\t"  \
    "mv %[npc], t0  # uretaddr \n\t"  \
    : [npc]"=r"(npc)\
    :  \
); 

//设置uretaddr寄存器的值为将要跳转的地址
#define SetNPC(npc) asm volatile( \
    "mv t0, %[npc]  # uretaddr \n\t"  \
    "addi x0, t0, 5 \n\t"  \
    : \
    :[npc]"r"(npc)  \
); 

//根据uretaddr寄存器发生跳转
#define URet() asm volatile( \
    "addi x0, x0, 128  # uret \n\t"  \
); 

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


//设置硬件一些TempReg的值。共提供了三个临时寄存器，可以用于读写
#define SetTempReg(t1, t2, t3) asm volatile( \
    "mv t0, %[rtemp1]  # tag \n\t"  \
    "addi x0, t0, 7 \n\t"  \
    "mv t0, %[rtemp2]  # exit \n\t"  \
    "addi x0, t0, 8 \n\t"  \
    "mv t0, %[rtemp3]  # maxinst \n\t"  \
    "addi x0, t0, 9 \n\t"  \
    : \
    :[rtemp1]"r"(t1), [rtemp2]"r"(t2), [rtemp3]"r"(t3)  \
);

//保存和恢复sp, gp, tp, fp ,ra
//基址由tempReg2寄存器决定
#define Load_necessary_exit() asm volatile( \
    "addi x0, t0, 1029 \n\t"   \
    "ld sp,8*0(t0)  \n\t"   \
    "ld gp,8*1(t0)  \n\t"   \
    "ld tp,8*2(t0)  \n\t"   \
    "ld fp,8*3(t0)  \n\t"   \
    "ld ra,8*4(t0)  \n\t"   \
); 

#define Save_necessary_exit() asm volatile( \
    "addi x0, t0, 1029 \n\t"   \
    "sd gp,8*1(t0)  \n\t"   \
    "sd tp,8*2(t0)  \n\t"   \
    "sd fp,8*3(t0)  \n\t"   \
    "sd ra,8*4(t0)  \n\t"   \
); 

//保存和恢复32个定点寄存器
//由a0作为基址寄存器，其中a0的值首先保存到uscratch寄存器中（额外增加的）
//基址由tempReg1寄存器决定
#define Save_int_regs_exit() asm volatile( \
    "addi x0, a0, 4  # write a0 to uscratch \n\t"   \
    "addi x0, a0, 1028 # read temp1 to a0 \n\t"   \
    "sd x1,8*1(a0)  \n\t"   \
    "sd x2,8*2(a0)  \n\t"   \
    "sd x3,8*3(a0)  \n\t"   \
    "sd x4,8*4(a0)  \n\t"   \
    "sd x5,8*5(a0)  \n\t"   \
    "sd x6,8*6(a0)  \n\t"   \
    "sd x7,8*7(a0)  \n\t"   \
    "sd x8,8*8(a0)  \n\t"   \
    "sd x9,8*9(a0)  \n\t"   \
    "sd x10,8*10(a0)  \n\t"   \
    "sd x11,8*11(a0)  \n\t"   \
    "sd x12,8*12(a0)  \n\t"   \
    "sd x13,8*13(a0)  \n\t"   \
    "sd x14,8*14(a0)  \n\t"   \
    "sd x15,8*15(a0)  \n\t"   \
    "sd x16,8*16(a0)  \n\t"   \
    "sd x17,8*17(a0)  \n\t"   \
    "sd x18,8*18(a0)  \n\t"   \
    "sd x19,8*19(a0)  \n\t"   \
    "sd x20,8*20(a0)  \n\t"   \
    "sd x21,8*21(a0)  \n\t"   \
    "sd x22,8*22(a0)  \n\t"   \
    "sd x23,8*23(a0)  \n\t"   \
    "sd x24,8*24(a0)  \n\t"   \
    "sd x25,8*25(a0)  \n\t"   \
    "sd x26,8*26(a0)  \n\t"   \
    "sd x27,8*27(a0)  \n\t"   \
    "sd x28,8*28(a0)  \n\t"   \
    "sd x29,8*29(a0)  \n\t"   \
    "sd x30,8*30(a0)  \n\t"   \
    "sd x31,8*31(a0)  \n\t"   \
);  

#define Load_int_regs_exit() asm volatile( \
    "addi x0, a0, 1028 # read temp1 to a0 \n\t"   \
    "ld x1,8*1(a0)  \n\t"   \
    "ld x2,8*2(a0)  \n\t"   \
    "ld x3,8*3(a0)  \n\t"   \
    "ld x4,8*4(a0)  \n\t"   \
    "ld x5,8*5(a0)  \n\t"   \
    "ld x6,8*6(a0)  \n\t"   \
    "ld x7,8*7(a0)  \n\t"   \
    "ld x8,8*8(a0)  \n\t"   \
    "ld x9,8*9(a0)  \n\t"   \
    "ld x11,8*11(a0)  \n\t"   \
    "ld x12,8*12(a0)  \n\t"   \
    "ld x13,8*13(a0)  \n\t"   \
    "ld x14,8*14(a0)  \n\t"   \
    "ld x15,8*15(a0)  \n\t"   \
    "ld x16,8*16(a0)  \n\t"   \
    "ld x17,8*17(a0)  \n\t"   \
    "ld x18,8*18(a0)  \n\t"   \
    "ld x19,8*19(a0)  \n\t"   \
    "ld x20,8*20(a0)  \n\t"   \
    "ld x21,8*21(a0)  \n\t"   \
    "ld x22,8*22(a0)  \n\t"   \
    "ld x23,8*23(a0)  \n\t"   \
    "ld x24,8*24(a0)  \n\t"   \
    "ld x25,8*25(a0)  \n\t"   \
    "ld x26,8*26(a0)  \n\t"   \
    "ld x27,8*27(a0)  \n\t"   \
    "ld x28,8*28(a0)  \n\t"   \
    "ld x29,8*29(a0)  \n\t"   \
    "ld x30,8*30(a0)  \n\t"   \
    "ld x31,8*31(a0)  \n\t"   \
    "addi x0, a0, 1026  \n\t"   \
); 


//-------------------------------------------------------------------
//将所有计数器的值设置为0
#define RESET_COUNTER asm volatile( \
	" andi x0, t0, 64      \n\t" \
);

//读取第n个计数器的值
unsigned long long read_counter(int n)
{
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

#endif