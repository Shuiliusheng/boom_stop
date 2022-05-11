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
    - 增加了64个计数器，用于统计硬件中的事件 (可以通过修改boom/parameter.scala中的subECounterNum进行扩展)
    - 限制1：仅限于统计progTag=0x1234567进程的事件
    - 限制2：用户自行设置需要统计的事件级别，
        - SetCounterLevel(0) : 仅统计用户级事件的数量
        - SetCounterLevel(1) : user + super
        - SetCounterLevel(3) : user + super + machine
    - 使用方法：
        - reset: andi x0, rs1, 1024
        - 读取计数器: andi x0, rs1, 512-1023  #用于读取第1-n个计数器到rs1寄存器中
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

4. 代码的修改标注：
    - Enable_PerfCounter_Support：计数器相关的代码修改标注
    - Enable_MaxInsts_Support：支持maxinsts相关的代码修改标注
    - Enable_Ckpt_Support：支持ckpt相关的代码修改标注

5. 目录介绍
    - boom_scala: 以SonicBOOM的cpu为例进行了相应功能的硬件支持
    - linux_arch_riscv: 修改linux，以支持在进程切换时恢复和保存processTag
    - fpga_ckpt：为了支持ckpt所需要的文件，主要从gem5_riscv_ckpt目录下获取
        - 注意：和gem5_riscv_ckpt的主要不同：修改了临时寄存器操作的汇编指令
        - gem5_configs：原本gem5的config目录，但是增加了一些参数，用于支持控制ckpt文件的生成
        - gem5_create: 用于生成ckpt所修改的gem5原本src目录
        - gem5_restore： 用于恢复ckpt所修改的gem5原本src目录
        - build_create & build_restore：仅用于做docker的目录映射，用于映射docker中gem5的build目录
        - podman_create.sh podman_restore.sh：用于启动docker
        - fpga_support：主要修改了rocket/subSystem/config.scala文件，用于扩大memsize，用于支持pk的运行
        - ckptinfo: 生成和恢复checkpoint文件的主要功能
            - createckpt: 用于根据gem5生成的log来生成ckpt
            - readckpt: 加载和恢复ckpt的加载器源码
            - tempckpt：仅用于读取ckpt的一些信息
            - test：存放一些test文件而已
            - info：存放生成的ckpt文件
    - riscv-pk：测试verilator生成的模拟器时所使用的pk

5. 目前63个计数器对应的事件
    0	cycles
    1	insts
    2	icache_access
    3	icache_hit
    4	icache_to_L2
    5	itlb_access
    6	itlb_hit
    7	itlb_to_ptw
    8	npc_from_f1
    9	npc_from_f2
    10	npc_from_f3
    11	npc_from_core
    12	fb_out_zero
    13	fb_out_full
    14	fb_out_notFull
    15	dec_out_zero
    16	dec_out_full
    17	dec_out_notFull
    18	brmask_stall
    19	rename_stall
    20	dis_out_zero
    21	dis_out_full
    22	dis_out_notFull
    23	dis_ldq_stall
    24	dis_stq_stall
    25	dis_rob_stall
    26	iss_uop_num
    27	iss_val_zero
    28	iss_val_full
    29	iss_val_notFull
    30	spec_miss_issuop
    31	exe_is_ld
    32	exe_is_st
    33	dtlb_access
    34	dtlb_miss_num
    35	dtlb_to_ptw
    36	dcache_access
    37	dcache_nack_num
    38	dcache_to_L2
    39	exe_is_br
    40	exe_is_jalr
    41	exe_is_ret
    42	exe_is_jalrcall
    43	exe_misp_br
    44	exe_misp_jalr
    45	exe_misp_ret
    46	exe_misp_jalrcall
    47	com_is_ld
    48	com_is_st
    49	com_is_br
    50	com_is_jalr
    51	com_is_ret
    52	com_is_jalrcall
    53	com_misp_br
    54	com_misp_jalr
    55	com_misp_ret
    56	com_misp_jalrcall
    57	l2tlb_miss
    58	misalign_excpt
    59	lstd_pagefault
    60	fetch_pagefault
    61	mini_exception
    62	rollback_cycles