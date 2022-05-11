### 使用方法
1. 参照example中的样例
    - define.h中定义了控制寄存器和计数器操作的宏
        - 基本不需要改动
    - ctrl.h则包括了具体设置控制寄存器和读计数器的函数
    - 使用时包含ctrl.h，同时修改init_start函数，exit_func函数和exit_record函数即可
        - init_start：用于设置一些控制寄存器
            - SetTempRegs(t1, t2, t3)：保存一些用于存储寄存器值的数组基址
            - Save_Basic_Regs()：保存一些基本的寄存器值，fp, tp, ra等，其中sp被设置为necessaryRegs[400]
            - SetCounterLevel("0"): 设置计数器要统计哪个级别的事件
                - 0：只统计user模式下的事件
                - 1：统计super+user模式的事件
                - 3：统计所有事件
            - SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst)
                - procTag：设置当前进程的标记，只有当标记为0x1234567时，才能触发各种事件和统计计数器
                - exitFucAddr：设置到达最大指令数时程序跳转的目标地址（exit_fuc函数的Save_ALLIntRegs第一条指令）
                - maxinst：设置从该条指令之后，程序执行多少条用户态指令时将跳转到exitFucAddr地址
                - warmupinst：设置从该条指令之后，程序执行多少条用户态指令重置所有计数器的值，然后再执行maxinst
            - RESET_COUNTER： 重置所有的计数器为0
            - ReadCounter16(baseaddr, startcouter)：从startcounter的计数器开始，读取16个计数器的值，存储到baseaddr地址对应的存储区域。startcounter从0开始

        - exit_func：执行到最大指令数时会跳转到该函数执行
            - Save_ALLIntRegs()：存储跳转到该地址时所有寄存器的值到intregs数组中。intregs数组的基址由SetTempRegs记录在临时寄存器中
            - Load_Basic_Regs(): 加载一些Save_Basic_Regs保存的寄存器值
            - GetNPC(npc) & SetNPC(npc)：获取跳转到exit_fuc之前的npc，并将其记录在uretaddr特殊寄存器中
            - write(1, str_temp, strlen(str_temp))：使用write显示而不是printf，以避免和主程序的printf冲突
            - SetCtrlReg(procTag, exitFucAddr, maxinst, warmupinst)：
                - 如果希望回到主程序继续执行，并且执行固定的指令数，则进行设置；
                - Load_ALLIntRegs() & URet()：恢复主程序的寄存器值，跳转回去继续执行
                - 否则，不需要设置，并且使用exit结束执行即可
        
        - exit_record：用于读取计数器的值，并且输出
            - 需要注意：输出使用write函数完成，以避免和原本的printf冲突
            - 读完之后可以重置所有计数器的值