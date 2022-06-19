### 在原本readckpt的基础上增加和修改的内容
1. define.h: 用于定义一些maxinst和warmup功能需要的内联汇编
    - 具体的硬件实现和介绍，请参考boom_stop
    - 支持最大指令数的相关内联汇编定义
    ```c
      //增加的指令，用于设置硬件寄存器，指示当前进程标志
      #define SetProcTag(srcreg)
      //增加的指令，用于设置硬件寄存器，指示执行到最大指令数时的跳转地址
      #define SetExitFuncAddr(srcreg)   
      //增加的指令，用于设置硬件寄存器，指示要执行的最大指令数
      #define SetMaxInsts(srcreg)       
      //增加的指令，用于设置硬件寄存器，uscatch作为临时寄存器使用
      #define SetUScratch(srcreg)       
      //增加的指令，用于设置硬件寄存器，设置uret寄存器，用于之后uret指令使用，完成跳转
      #define SetURetAddr(srcreg)       
      //增加的指令，用于设置硬件寄存器，硬件计数器统计的事件级别
      #define SetMaxPriv(srcreg)        
      //增加的指令，用于设置硬件寄存器，设置临时寄存器的值（提供了四个临时寄存器）
      #define SetTempReg(srcreg, rtemp) 
      //增加的指令，用于设置硬件寄存器，指示计数器开始工作的指令数，即warmup的指令数
      #define SetStartInsts(srcreg)     
      //增加的指令，用于获取硬件寄存器，即获取设置的进程标记
      #define GetProcTag(dstreg)        
      //增加的指令，用于获取uscatch硬件寄存器，
      #define GetUScratch(dstreg)       
      //增加的指令，用于获取发生maxinst跳转之前的指令pc
      #define GetExitNPC(dstreg)        
      //增加的指令，用于获取临时寄存器的值
      #define GetTempReg(dstreg, rtemp) 
      //增加的指令，用于搭配SetURetAddr完成跳转到任意地址
      #define URet() asm volatile( "addi x0, x0, 128  # uret \n\t" ); 

      //利用GetExitNPC和SetURetAddr，完成对发生maxinst退出时的指令pc获取和之后的跳转
      #define GetNPC(npc)
      #define SetNPC(npc)
      //设置计数器的事件统计级别0:user, 1: user+super, 3: all
      #define SetCounterLevel(priv)
      //设置进程tag，退出函数入口地址，最大指令数和warmup的指令数
      #define SetCtrlReg(tag, exitFucAddr, maxinst, startinst)
      //设置三个临时寄存器的值
      #define SetTempRegs(t1, t2, t3)
      //保存和恢复一些基本临时寄存器的值，sp会单独设置成另一块内存区域，以避免修改栈
      #define Load_Basic_Regs()
      #define Save_Basic_Regs()
      //保存和恢复所有寄存器的值，使用uscratch作为临时寄存器，保存a0的值，然后以a0为基址进行保存和恢复
      #define Regs_Operations(Op)
      #define Save_ALLIntRegs()
      #define Load_ALLIntRegs()
    ```
    - 支持操作硬件计数器的相关内联汇编定义
    ```c
      //重置所有计数器的值为0
      #define RESET_COUNTER
      //获取计数器的值
      #define GetCounter(basereg, dstreg, start, n)
      #define ReadCounter8(base, start)
      #define ReadCounter16(base, start)
    ```

2. ctrl.h: 提供的一些c函数，用于完成对maxinst和warmup，以及exitfuc入口地址的设置
    - 全局变量定义
      ```c
        unsigned long long intregs[32];
        unsigned long long necessaryRegs[1000]; //用于作为新的栈空间
        unsigned long long npc=0;
        unsigned long long procTag=0x1234567;   //要求必须设置为该数字才能够使用计数器的值
        unsigned long long maxinst=1000000, warmupinst=0;
        extern unsigned long long __exitFucAddr;  //exit_fuc的入口地址，用于作为maxinst退出时的跳转函数
        char str_temp[300];

        //作为统计事件的变量
        unsigned long long startcycle = 0, endcycle = 0;
        unsigned long long startinst = 0, endinst = 0;
        unsigned long long exit_counters[64];
      ```
    - 函数定义
      - void exit_record(): 函数退出时读取硬件计数器的值
      - void exit_fuc(): 退出函数的处理
        - 保存当前所有整数寄存器值（不会修改浮点寄存器的情况时，则不需要保存浮点寄存器）
        - 恢复部分基本的寄存器（主要是sp寄存器）
        - 获取进入exit_fuc之前最后一条指令的pc
        - 处理一些事件处理，例如读取计数器等之类的，此时需要使用write函数进行输出，printf不行
        - 如果需要回到原函数继续执行，需要恢复寄存器的状态，跳转回原来的程序执行

      void init_start(unsigned long long max_inst, unsigned long long warmup_inst);
        - 保存一些必要的寄存器到necessaryRegs中，并且单独设置sp寄存器为新的一块区域
        - 设置要统计事件的级别
        - 重置所有计数器的值
        - 利用setCtrlReg设置进程标记，退出函数入口地址，最大指令数和warmup指令数
        - 在read_ckptinfo函数最终要恢复寄存器之前，调用该函数，设置maxinst和warmup信息
    