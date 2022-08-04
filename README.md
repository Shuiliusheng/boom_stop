### RISCV-BOOM支持基于不同的硬件事件进行采样
1. 增加额外的硬件寄存器控制硬件计数器和事件采样的触发，同时增加配套的指令操作这些寄存器
    - addi x0, rs1, #tag
    - 设置硬件寄存器
      - tag = 1 : 设置进程独有标识寄存器procTag, procTag = Reg[rs1]
        - 操作系统在发生进程切换时保存每个进程的procTag
        - 子进程在生成时继承父进程的procTag
      - tag = 2 : 设置跳转采样处理函数的入口地址寄存器, sampleFuncAddr = Reg[rs1]
        - 当sampleFuncAddr设置为0时，则采样事件会触发系统调用，此时要求由软件接管该系统调用
        - 其余情况，处理器则跳转到sampleFuncAddr指定的代码开始执行
      - tag = 3 : 设置sampleEventSel, sampleEventSel = Reg[rs1] % 8
        - 设置要采样的事件，默认为0，选择以指令数进行采样
      - tag = 4 : 设置maxEventNum, maxEventNum = Reg[rs1]
        - 设置采样事件的最大值，当事件到达该值时触发采样
      - tag = 5 : 设置warmupInstNum, warmupInstNum = Reg[rs1]
        - 设置warmup的指令数，当程序的用户态指令数达到该值时重置所有硬件计数器的内容
      - tag = 6 : 设置pfc_maxPriv寄存器,  pfc_maxPriv = Reg[rs1] % 4
        - 设置计数器要采集各种事件的级别：0(user) /1(user+super) /3(all level)  
      - tag = 7 : 设置pfc_enable寄存器,  pfc_enable = Reg[rs1] % 1
        - 设置计数器当前是否工作
        - 当发生采样时，在处理器跳转触发采样时，该计数器设置为0，所有计数器停止工作
      - tag = 8 : 设置sampleHappen寄存器,  sampleHappen = Reg[rs1] 
        - 当发生采样并处理器触发采样跳转时，该计数器被设置，用于指明当前的系统调用或者跳转是由采样事件而引发的，之后由软件来决定当前如何处理
        - 该计数器之后可以用于设置采样发生的事件原因
      - tag = 9 :  设置temp1寄存器, temp1 = Reg[rs1]
      - tag = 10 : 设置temp2寄存器, temp2 = Reg[rs1]
      - tag = 11 : 设置temp3寄存器, temp3 = Reg[rs1]
      - tag = 12 : 设置temp4寄存器, temp3 = Reg[rs1]
        - 临时寄存器作用：主要用于当sampleFuncAddr不为零时，用户程序需要自行处理保存和恢复上下文以及处理跳转的功能。利用临时寄存器，可以避免修改用户寄存器
    - 读取硬件计数器
      - tag = 33 : 读取procTag, Reg[rs1] = procTag
      - tag = 34 : 读取sampleHappen寄存器, Reg[rs1] = sampleHappen
      - tag = 35 : 读取userExitPC寄存器, Reg[rs1] = userExitPC
        - 该寄存器存储着当处理器处理采样时跳转到sampleFuncAddr之前最后一条指令的地址
      - tag = 36 : 读取temp1寄存器, Reg[rs1] = temp1
      - tag = 37 : 读取temp2寄存器, Reg[rs1] = temp2
      - tag = 38 : 读取temp3寄存器, Reg[rs1] = temp3
      - tag = 39 : 读取temp4寄存器, Reg[rs1] = temp4
      - tag = 41 : 读取sampleFuncAddr寄存器, Reg[rs1] = sampleFuncAddr
      - tag = 42 : 读取maxEventNum寄存器, Reg[rs1] = maxEventNum
      - tag = 43 : 读取nowEventNum寄存器, Reg[rs1] = nowEventNum
      - tag = 44 : 读取sampleEventSel寄存器, Reg[rs1] = sampleEventSel
      - tag = 45 : 读取pfc_maxPriv寄存器, Reg[rs1] = pfc_maxPriv
    - 特殊的指令功能
      - tag = 64 : URet, 根据temp寄存器发生跳转, 其中rs1位域的内容决定要根据哪一个临时寄存器跳转
        - rs1 = 0 : jmp to temp1寄存器
        - rs1 = 2 : jmp to temp2寄存器
      - tag = 127: 重置所有计数器的值


2. 性能寄存器的读取和使用
    - 增加了64个计数器，用于统计硬件中的事件 (可以通过修改boom/parameter.scala中的subECounterNum进行扩展)
    - 限制1：仅限于统计progTag=0x1234567进程的事件
    - 限制2：用户自行设置需要统计的事件级别：设置pfc_maxPriv寄存器的值来决定统计事件的级别
    - 使用方法：
        - reset: addi x0, x0, 127
        - 读取计数器: addi x0, rs1, 128-255  #用于读取第1-n个计数器到rs1寄存器中
    - 搭配使用：
        - 程序在设置warmupInstNum后，将在执行到该位置时重置所有计数器
  
3. BOOM代码的修改标注：
    - Enable_PerfCounter_Support：计数器相关的代码修改标注
    - Enable_Sample_Support：支持maxinsts相关的代码修改标注

4. 用户自行处理采样事件
    - 主要思路：在程序初始阶段，设置需要的硬件寄存器，完成对采样的设置，同时设置sampleFuncAddr为用户定义的采样函数的入口地址。自定义的采样函数完成对上下文的保存/恢复/和返回程序原先位置继续执行的工作。
    - 具体设计：
      - 利用__attribute((constructor))声明，完成在程序起始位置处的寄存器设置
        - 利用全局变量tempStackMem，分配一块内存空间，完成一些工作：采样函数起始位置保存当前上下文寄存器，存储采用函数需要使用的寄存器的内容；作为采样函数的栈空间使用。
        - tempStackMem的起始地址可以使用la伪指令获取，用于在sampleFunc的入口处获取到指定寄存器，然后保存上下文
        - 在tempStackMem保存当前一些必要的寄存器(gp, tp, ra)，同时设置栈基址和相关寄存器
          - sp = &tempStackMem[2048]; s0 = &tempStackMem[2560];
          - 设置栈空间的目的是防止采样函数修改原本程序所使用的栈空间内容
        - 设置计数器统计事件的级别pfc_maxPriv，同时开启硬件计数器pfc_enable，重置所有计数器为0
        - 设置procTag, sampleFucAddr, maxevent, warmupinst, eventsel
      - 采样函数的设计
        - 利用asm volatile("__sampleFucAddr: ")的方式，确定采样函数实际入口地址
        - 将当前所有寄存器的内容保存到tempStackMem中
          - 利用temp0寄存器暂存a0寄存器中的内容，然后使用a0寄存器作为基址寄存器
          - 利用la指令，获取tempStackMem的基址，将其赋给a0寄存器
          - 利用store指令完成对当前除了a0寄存器以外所有整数寄存器的保存（sampleFunc不包括浮点操作）
          - 读取temp0寄存器中a0的内容，保存到tempStackMem对应的位置中即可
        - 获取采样函数需要使用的一些寄存器
          - 利用la指令，获取tempStackMem的基址，将其赋给a0寄存器
          - 利用load指令恢复gp, tp, ra, sp, s0寄存器
        - 完成采样事件的自定义操作，包括读取计数器内容，保存到文件中
        - 跳转回到原本程序，继续执行
          - 重置所有计数器，开启硬件计数器pfc_enable，设置procTag, sampleFucAddr, maxevent, warmupinst, eventsel
          - 获取跳转到sampleFunc之前的指令地址exitPC（引发处理器跳转的指令地址）
          - 将exitPC写入到temp0寄存器中
          - 根据tempStackMem，恢复程序原本所有寄存器的内容.（此时不在需要临时寄存器的辅助）
          - 利用URet的指令功能，跳转到temp0寄存器的地址，回到程序继续执行
      - 注意事项：
        - 为了避免la a0, tempStackMem指令转换为gp+imm的形式，因此在tempStackMem之前声明了一块全局内存，使得tempStackMem地址操作gp+4KB
        - 采样函数中避免使用printf函数，因为该函数会在执行过程中加锁，因此会出现死锁的情况。为此，可以使用sprintf+write的组合完成输出/保存到文件中的操作

5. 利用ptrace捕获并处理采样事件
    - 主要思路：
      - 提供一个工具避免修改要采样的程序
      - 利用execv的支持来启动一个新的程序，同时在启动之前完成对procTag的设置
      - 硬件上在采样发生时引发系统调用，并且设置sampleHappen寄存器，以此标记该系统调用是采样事件导致的
      - 软件上利用ptrace追踪程序的系统调用，并根据sampleHappen判断是否发生了采样事件。当发生时，获取计数器的内容，完成采样。同时设置下一次发生采样的控制寄存器
    - 具体设计：
      - 启动测试程序
        - 利用fork生成子进程
        - 在子进程中设置procTag为0x1234567，并且将sampleFuncAddr设置为0，此时硬件将会在发生采样时引发系统调用
        - 利用execv启动测试程序
      - 获取采样事件，完成采样
        - 在父进程中，利用ptrace追踪子进程的系统调用
        - 当第一次执行brk系统调用时，设置采样控制寄存器，启动采样
          - 此时是程序刚开始执行的时候
        - 在系统调用执行之前，读取sampleHappen寄存器。如果该寄存器为1，则表明当前系统调用为采样事件引发的。
          - 读取所有整数寄存器和PC，
          - 修改系统调用号(修改a7寄存器)，将其设置为尚未实现的系统调用，以避免系统调用执行修改内存
          - 执行采样函数：读取计数器等操作
        - 在系统调用执行之后，将所有寄存器设置为进入系统调用之前的状态。同时修改pc=pc-4，以使得系统调用结束后跳转回到触发采样的指令，重新执行

6. 具体使用: example中提供了不同的示例


7. 目录介绍
    - boom_scala: 以SonicBOOM的cpu为例进行了相应功能的硬件支持
    - linux_arch_riscv: 修改linux，以支持在进程切换时恢复和保存processTag，同时子进程的threadtag由父进程给出
    - example: 使用示例
      - ptrace：利用ptrace方式实现的采样工具，支持对任意的执行程序进行事件采样设置，并且不需要修改代码
      - self_handle: 单独自行设置采样控制和采样的处理，所有代码需要包含到要采样的程序中，共提供了两种方案
        - binary: 将采样设置和采样函数的处理，单独生成.o文件。要采样的程序编译链接时将其链在一起即可。
        - include_files: 提供采样设置和采样函数的头文件，直接包含到程序中，显示在某些地方调用即可。
    - log_process: 提供了一些python脚本用于处理采样收集到的数据，将其整理为csv的形式
    - riscv-pk：测试verilator生成的模拟器时所使用的pk
    - test: 用于测试正确性时的一些测试文件
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
    

8. 目前63个计数器对应的事件
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