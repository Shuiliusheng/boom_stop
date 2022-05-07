### 扩展boom的源码 ####
1. 修改rocket-chip/src/main/scala/subsystem/Configs.scala文件, 扩展默认的mem_size
   - 原本memsize = 0x10000000 -> 0x40000000
   - 通过扩展memsize, 以使得模拟过程中可用的栈空间和mmap的空间变大
   - (创建ckpt时是缩小stack和mmap的空间, 从而使得重新恢复ckpt时能够使用mmap直接将stack和其他区域进行映射)

2. 修改boom v3的源码, 以支持临时寄存器的读写和跳转
   - 增加四个临时寄存器: 将原本32个逻辑寄存器扩展到36个, 其余不变
   - 单独处理读/写/跳转临时寄存器的指令的译码逻辑, 将ldst/lrs2/lrs1字段单独设置为临时寄存器(r32, r33, r34, r35)
   - 读取临时寄存器：

     - 用于读取临时寄存器中的值到寄存器中，主要用于在保存完运行时状态的结尾恢复某个寄存器的值，搭配write rtemp使用

     ```assembly
     # and x0, rd, rtemp
     # REG[rd] = REG[rtemp]
     and x0, a1, x2  #read rtemp2 to reg a1
     ```

   - 写临时寄存器：

     - 用于在保存运行时状态时，暂存某个寄存器的值，搭配read rtemp使用
     - 用于设置某些固定的跳转地址，搭配jmp rtemp使用

     ```c
     // sll x0, rs1, rtemp
     // REG[rtemp] = REG[rs1]
     #define WriteTemp(num, val) asm volatile( \
         "mv t0, %[data] \n\t"   \
         "sll x0, t0, x" num " \n\t"   \
         "fence \n\t"   \
         :  \
         :[data]"r"(val)  \
     );
     ```

   - 根据临时寄存器跳转

     - 用于跳转到需要执行程序的起始地址（使用pc+imm无法跳到足够远的地方）
     - 用于接管系统调用时，跳转到固定的系统调用处理程序入口（提前将执行程序的ecall替换为jmptemp指令）

     ```c
     // or x0, x0, rtemp
     // jump to REG[rtemp]
     #define JmpTemp(num) asm volatile( \
         "xor x0, x0, x" num " \n\t"   \
     );
     ```