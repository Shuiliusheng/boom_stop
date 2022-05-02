### RISCV-BOOM执行固定指令数停止
1. 设置进程独有标识
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


    - tag = 128 : uret功能，根据uretaddr进行跳转


    - tag = 1025 : 读取processTag, Reg[rs1] = processTag
    - tag = 1026 : 读取uscratch寄存器, Reg[rs1] = uscratch
    - tag = 1027 : 读取exitNPC寄存器, Reg[rs1] = exitNPC

    - tag = 1028 : 读取temp1寄存器, Reg[rs1] = temp1
    - tag = 1029 : 读取temp2寄存器, Reg[rs1] = temp2
    - tag = 1030 : 读取temp3寄存器, Reg[rs1] = temp3


2. 硬件统计进程执行指令数（用户态）
3. 硬件跳转到固定的函数入口，结束执行


##  note
- 如果想测试riscv-test，就需要将core.scala的csr.io.status.prv === 0.U 改为 3.U
    - 0.U: user, 3.U: machine