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

3. 具体使用
    - 包含example/pfc_asm.h头文件
    - start_record函数会自动在main函数之前被调用执行，因此可以在该函数对一些特殊寄存器进行设置
    - exit_fuc函数是给出的一个示例函数，用于处理达到最大指令数的信息收集和输出

