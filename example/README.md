### 使用方法

1. 在源程序中包含ctrl.h，重新使用build.sh编译即可
    - ctrl.h中init_start函数会自动在main函数之前被调用执行
    - init_start函数具体完成的任务：
        - SetTempReg(t1, t2, t3) & Save_necessary(): 用于保存一些必要的寄存器值， 同时将necessRegs[400]作为新的sp，用于临时使用
        - SetCounterLevel(0): 用于设置计数器需要统计哪些级别的时间，0表示仅统计用户态，1表示统计user+super，3表示user+super+machine
        - RESET_COUNTER：用于将所有计数器重设为0
        - SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst)：用于控制程序执行
            - procTag: 设置当前进程的TAG，只有当该TAG为0x1234567时，才能够统计事件，自动退出等
            - exitFucAddr: 当进程达到最大执行执行数时，跳转的入口地址，通常指向exit_fuc的入口。当为0时。不发生任何跳转；
            - maxinst：设置从当前开始，程序能够执行的最大指令数（用户态指令）。如果为0，则无任何影响；
            - warmupinst：设置从当前开始，执行多少条指令之后，cpu自动reset所有计数器的值
    - maxinst可以在运行时输入，此时只需要在init_start中增加输入函数即可
    - build.sh脚本中，会编译两次，第一次编译完成后，自动获取exitFuc的入口地址，然后替换ctrl.h文件中的变量值，然后进行第二次编译
    - ctrl.h中的内容基本不需要改动，define.h中主要定义了一些汇编的宏定义

2. ctrl.h中exit_fuc函数介绍：
    - 需要注意：exitFucAddr并不是指向该函数的第一条指令，而是Save_ALLIntRegs的第一条指令
    
    - 信息保存：如果不需要跳回到进程继续执行，可以不需要
        - Save_ALLIntRegs：将当前进程的寄存器信息保存下来，类似于正常的异常处理程序的入口
        - Load_Basic_Regs: 将一些必要的寄存器值恢复到最开始的内容，和Save_Basic_Regs对应。为了避免exit_fuc修改栈的内容，因此sp会被设置成新的值（necessRegs[400]）
        - GetNPC(npc) & SetNPC(npc)：获取进程进入exit_fuc之前程序将要执行的下一条指令PC，同时将其设置在硬件临时寄存器中，用于之后跳转回去；
    
    - 信息显示：
        - write函数：使用printf函数可能会导致和源程序的printf函数冲突，可能引发临界区资源的冲突而卡死
        - exit_record：用于获取当前计数器的值，并进行输出（以json的格式输出，便于之后处理）
    
    - 恢复执行：如果不需要恢复，直接使用exit结束执行即可
        - SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst)：重新设置下一次触发退出的条件
        - Load_ALLIntRegs：将Save_ALLIntRegs保存的内容恢复回去
        - URet：根据SetNPC的内容进行跳转


2. ctrl.h中exit_record函数介绍：   
    - 使用define.h中定义的read_counter函数读取每个计数器的值
    - 使用sprintf+write，输出计数器的值
    - RESET_COUNTER：重设计数器的值为0

3. define.h中定义的函数介绍：
    - GetNPC(npc)：获取发生exit实现时下一条指令的pc
    - SetNPC(npc)：设置uretaddr寄存器的值为将要跳转的地址
    - URet()：根据uretaddr寄存器发生跳转
    - SetCounterLevel(priv)：设置计数器需要统计的级别
        - 0：仅统计user的事件
        - 1：仅统计user+super的事件
        - 3：统计所有事件
    - SetCtrlReg(tag, exitFucAddr, maxinst, startinst)
        - 设置硬件一些临时寄存器的值，以此控制程序执行
        - processTag：设置为tag的值，仅当为0x1234567时才进行各种操作，包括退出和事件计数
        - exitFuncAddr：设置为exitFucAddr，仅当不为0并且procMaxInsts也不为0时有效。控制退出事件时的跳转目标地址。
        - procMaxInsts：设置为maxinst，控制当进程的用户态指令数达到maxinsts时，发生退出情况
        - startInsts：设置为startinst，控制当进程的用户态指令数达到startinst时，重置计数器

    - SetTempRegs(t1, t2, t3) 
        - 设置硬件一些TempReg的值。共提供了三个临时寄存器，可以用于读写
        - 主要用于存储：intregs和necessaryRegs的起始地址，用于之后程序保存寄存器的值
    
    - Load_Basic_Regs() & Save_Basic_Regs()
        - 保存和恢复sp, gp, tp, fp ,ra
        - 基址由tempReg2寄存器决定

    - Save_ALLIntRegs() & Load_ALLIntRegs()
        - 保存和恢复32个定点寄存器
        - 由a0作为基址寄存器，其中a0的值首先保存到uscratch寄存器中（额外增加的）
        - 基址由tempReg1寄存器决定

    - RESET_COUNTER
        - 将所有计数器的值设置为0
    - ReadCounter16 & ReadCounter8
        - 读取16个或者8个计数器到数组中
    - DEF_CSRR(cycle) & DEF_CSRR(instret)
        - 原本riscv获取cycle和指令数的csr指令

    - 一些基本的宏定义：
    ```c
        #define SetProcTag(srcreg)          "addi x0, " srcreg ", 1 \n\t"  
        #define SetExitFuncAddr(srcreg)     "addi x0, " srcreg ", 2 \n\t"  
        #define SetMaxInsts(srcreg)         "addi x0, " srcreg ", 3 \n\t"  
        #define SetUScratch(srcreg)         "addi x0, " srcreg ", 4 \n\t" 
        #define SetURetAddr(srcreg)         "addi x0, " srcreg ", 5 \n\t"  
        #define SetMaxPriv(srcreg)          "addi x0, " srcreg ", 6 \n\t"  
        #define SetTempReg(srcreg, rtemp)   "addi x0, " srcreg ", 7+" #rtemp " \n\t"  
        #define SetStartInsts(srcreg)       "addi x0, " srcreg ", 10 \n\t"   

        #define GetProcTag(dstreg)          "addi x0, " dstreg ", 1025 \n\t"  
        #define GetUScratch(dstreg)         "addi x0, " dstreg ", 1026 \n\t"  
        #define GetExitNPC(dstreg)          "addi x0, " dstreg ", 1027 \n\t"  
        #define GetTempReg(dstreg, rtemp)   "addi x0, " dstreg ", 1028+" #rtemp " \n\t"  

        #define URet() asm volatile( "addi x0, x0, 128  # uret \n\t" ); 

        #define GetPfcounter(dstreg, pfc)   "andi x0, " dstreg ", 32+" #pfc " \n\t"
        #define RESET_COUNTER asm volatile(" andi x0, t0, 64 \n\t" );
    ```
    