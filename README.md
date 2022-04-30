### RISCV-BOOM执行固定指令数停止
1. 设置进程独有标识
    - addi x0, rs1, #tag
    - tag = 1 : 设置进程独有标识, processTag = Reg[rs1]
    - tag = 2 : 设置跳转目标地址, exitFunc = Reg[rs1]
    - tag = 3 : 设置maxInst, maxInst = Reg[rs1]
2. 硬件统计进程执行指令数（用户态）
3. 硬件跳转到固定的函数入口，结束执行