### RISCV-BOOM执行固定指令数停止
1. 需要增加的额外指令功能支持
    - addi x0, rs1, #tag
    - tag = 1 : 设置进程独有标识, processTag = Reg[rs1]
    - tag = 2 : 设置跳转目标地址, exitFuncAddr = Reg[rs1]
    - tag = 3 : 设置maxInst, procMaxInsts = Reg[rs1]

    - tag = 4 : 设置uscratch寄存器, uscratch = Reg[rs1]
    - tag = 5 : 设置uretaddr寄存器, uretaddr = Reg[rs1]
    - tag = 6 : 设置maxpriv寄存器,  maxPriv = Reg[rs1] % 4

    - tag = 7 : 设置temp1寄存器, temp1 = Reg[rs1]
    - tag = 8 : 设置temp2寄存器, temp2 = Reg[rs1]
    - tag = 9 : 设置temp3寄存器, temp3 = Reg[rs1]
    - tag = 10 : 设置startInsts，用于控制程序执行到该数量的指令时，reset所有计数器


    - tag = 128 : uret功能，根据uretaddr进行跳转


    - tag = 1025 : 读取processTag, Reg[rs1] = processTag
    - tag = 1026 : 读取uscratch寄存器, Reg[rs1] = uscratch
    - tag = 1027 : 读取exitNPC寄存器, Reg[rs1] = exitNPC

    - tag = 1028 : 读取temp1寄存器, Reg[rs1] = temp1
    - tag = 1029 : 读取temp2寄存器, Reg[rs1] = temp2
    - tag = 1030 : 读取temp3寄存器, Reg[rs1] = temp3

    ```c
        #define SetProcTag(srcreg)          "addi x0, " srcreg ", 1 \n\t"  
        #define SetExitFuncAddr(srcreg)     "addi x0, " srcreg ", 2 \n\t"  
        #define SetMaxInsts(srcreg)         "addi x0, " srcreg ", 3 \n\t"  
        #define SetUScratch(srcreg)         "addi x0, " srcreg ", 4 \n\t" 
        #define SetURetAddr(srcreg)         "addi x0, " srcreg ", 5 \n\t"  
        #define SetMaxPriv(srcreg)          "addi x0, " srcreg ", 6 \n\t"  
        #define SetTempReg(srcreg, rtemp)   "addi x0, " srcreg ", 7+" rtemp " \n\t"  
        #define SetStartInsts(srcreg)       "addi x0, " srcreg ", 10 \n\t"   

        #define GetProcTag(dstreg)          "addi x0, " dstreg ", 1025 \n\t"  
        #define GetUScratch(dstreg)         "addi x0, " dstreg ", 1026 \n\t"  
        #define GetExitNPC(dstreg)          "addi x0, " dstreg ", 1027 \n\t"  
        #define GetTempReg(dstreg, rtemp)   "addi x0, " dstreg ", 1028+" rtemp " \n\t"  

        #define URet() asm volatile( "addi x0, x0, 128  # uret \n\t" ); 
    ```

1.1 性能寄存器的读取和使用
    - 增加了32个计数器，用于统计硬件中的事件
    - 限制1：仅限于统计progTag=0x1234567进程的事件
    - 限制2：用户自行设置需要统计的事件级别，
        - SetCounterLevel(0) : 仅统计用户级事件的数量
        - SetCounterLevel(1) : user + super
        - SetCounterLevel(3) : user + super + machine
    - 使用方法：
        - reset: andi x0, rs1, 64
        - 读取计数器: andi x0, rs1, 32-63  #用于读取第1-32个计数器到rs1寄存器中
    - 搭配使用：
        - 程序在设置startinsts后，将在执行到该位置时，reset所有计数器

1.2 ckpt的使用
    - fpga_ckpt中提供了生成和使用checkpoint的教程，主要是从项目gem5_riscv_ckpt中复制并简单修改过来的
    - 通过利用maxinst和warmup的机制，我们能够更加准确的控制ckpt的开始和结束
    - 通过搭配32个计数器，在warmup结束之后开始计数，将能够得到详细的ckpt内的事件信息
    - boom支持ckpt需要做出的改变：扩展32个逻辑寄存器到36个，即增加四个临时寄存器，用于读写和跳转使用
        - 读：ori x0, rs1, #4-7   # REG[rs1] = REG[0, 1, 2, 3]
        - 写：ori x0, rs1, #8-11  # REG[0, 1, 2, 3] = REG[rs1]
        - 跳转：ori x0, x0, #12-15  # jump to REG[0, 1, 2, 3]
        ```c
            #define WriteRTemp(srcreg, rtempnum) "ori x0, " srcreg ", 8+" rtempnum " \n\t"
            #define ReadRTemp(dstreg, rtempnum) "ori x0, " dstreg ", 4+" rtempnum " \n\t"
            #define JmpRTemp(rtempnum) "ori x0, x0, 12+" rtempnum " \n\t"
        ```


2. 主要思路
    - 在硬件中识别特定的procTag (0x1234567)，并且仅统计该标记的进程在用户态情况下执行的指令数
        - 启动程序时，使用(addi x0, t0, 1)设置进程标记： procTag = REG[t0]
        - 操作系统启动程序时，会自动将标记初始化为0
        - 在发生进程切换时，由操作系统保存上一进程的标记，并恢复下一进程的标记
            - 使用(addi x0, t0, 1025) 读取procTag到t0寄存器中
        - 为了统计指令数，增加计数器procRunningInsts，统计0x1234567标记的进程的指令数。使用时需要保证整个系统中仅存在一个标记为0x1234567的进程，因此不需要在进程切换时再保存该信息
    - 当procRunningInsts达到最大指令数procMaxInsts，并且最近两个周期都处于该0x1234567标记的进程的用户态中时，触发异常，并且此时选择跳转到硬件寄存器exitFuncAddr指定的位置（要求exitFuncAddr提前有值）
        - 启动程序使用，使用(addi x0, t0, 2)设置exitFuncAddr
        - 启动程序使用，使用(addi x0, t0, 3)设置procMaxInsts
        - exitFuncAddr为程序内某个函数的地址，用户处理退出时的信息打印等情况
    - 

3. 具体使用: example中存在一个示例
    - 将define.h 和 ctrl.h为必须存在的两个头文件
    - ctrl.h的start_record函数会自动在main函数之前被调用执行，因此可以在该函数对一些特殊寄存器进行设置
    - exit_fuc函数是给出的一个示例函数，用于处理达到最大指令数的信息收集和输出


4. 目前32个计数器对应的事件
    - event 0,  cycles
    - event 1,  commit_insts
    - event 2,  icache_access_num
    - event 3,  icache_miss_num
    - event 4,  itlb_miss_num
    - event 5,  npc_use_f1
    - event 6,  npc_use_f2
    - event 7,  npc_use_f3
    - event 8,  npc_use_core
    - event 9,  fb_no_output
    - event 10, fb_one_output
    - event 11, brmask_full_cycles 
    - event 12, rename_stall_insts
    - event 13, dis_ldq_stall_insts
    - event 14, dis_stq_stall_insts
    - event 15, dis_rob_stall_insts
    - event 16, issue_insts
    - event 17, misspec_iss_insts
    - event 18, dcache_access_num
    - event 19, dcache_nack_num
    - event 20, dcache_to_nextlevel
    - event 21, dtlb_access_num
    - event 22, dtlb_miss_num
    - event 23, dtlb_ptw_num
    - event 24, l2tlb_miss_num
    - event 25, misaligned & pagefault
    - event 26, mini_exception
    - event 27, rollback_cycles
    - event 28, commit_br_num
    - event 29, commit_jalr_num
    - event 30, branch_mis_num
    - event 31, jalr_mis_num