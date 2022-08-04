### 读取生成的ckpt文件，加载测试程序运行，需要硬件提供四个额外的逻辑寄存器，用于避免修改原本的寄存器
1. ckptinfo.h: ckpt加载器的头文件，用于定义一些struct, 汇编代码, 函数, extern变量
     - 常量定义
    ```c
        //定义RunningInfo所在的地址
        #define RunningInfoAddr 0x150000
        //根据RunningInfoAddr决定RunningInfo中intregs的位置
        #define StoreIntRegAddr "0x150028"
        //根据RunningInfoAddr决定RunningInfo中fpregs的位置
        #define StoreFpRegAddr  "0x150128"
        //根据RunningInfoAddr决定RunningInfo中oldregs的位置
        #define OldIntRegAddr   "0x150228"
        //由于benchmark代码段从0x200000开始，因此定义两个中间节点，用于从加载器跳转到距离benchmark最近的地方
        //用于实现跳转到takeOverSyscall函数入口的新指令，jmp Rtemp1, 根据临时寄存器1的值进行跳转
        #define ECall_Replace 0x00d06013
        //退出指令是否为系统调用指令，暂时没有使用
        #define Cause_ExitSysCall 1 
        #define Cause_ExitInst 2
    ```
    - 结构体定义
    ```c
        typedef struct{
            uint64_t addr; 
            uint64_t size;
        }MemRangeInfo; //存储区域信息，用于textrange和memrange

        typedef struct{
            uint64_t pc;    //ecall的pc
            uint64_t num;   //系统调用号
            uint64_t p0, p1, p2;    //系统调用参数
            uint64_t hasret, ret;   //返回值
            uint64_t bufaddr, data_offset, data_size; //修改的内存信息
        }SyscallInfo; //每一个系统调用包含的信息


        typedef struct{
            uint64_t exitpc;        //退出指令的pc
            uint64_t exit_cause;    //退出的原因，暂时不用
            uint64_t syscall_info_addr; //所有系统调用信息所在的内存地址
            uint64_t nowcallnum;    //当前ckpt执行了多少个系统调用
            uint64_t totalcallnum;  //一共ckpt包含多少系统调用
            uint64_t intregs[32];   //存储ckpt起点和执行过程中的寄存器信息
            uint64_t fpregs[32];
            uint64_t oldregs[32];   //存储加载器的整数寄存器信息，用于在takeOverSys函数中恢复加载器的执行环境
            uint64_t cycles;
            uint64_t insts;
            uint64_t lastcycles;
            uint64_t lastinsts;
            uint64_t startcycles;
            uint64_t startinsts;    //用于统计ckpt执行过程中的指令和周期信息
        }RunningInfo; 
        //ckpt恢复执行后需要的一些信息
        //设置在固定的地址RunningInfoAddr，从而能够找到这些信息
    ```
    - 内联汇编定义
    ```c
        //三种针对临时寄存器操作的汇编指令定义: 读/写/跳转
        #define WriteRTemp(srcreg, rtempnum) "ori x0, " srcreg ", 8+" #rtempnum " \n\t"
        #define ReadRTemp(dstreg, rtempnum) "ori x0, " dstreg ", 4+" #rtempnum " \n\t"
        #define JmpRTemp(num) asm volatile( "ori x0, x0, 12+" #rtempnum " \n\t" ); 
        #define WriteTemp(num, val)
        
        //恢复加载器执行环境的必要寄存器信息 sp, gp, tp, fp, 其余寄存器可以不用恢复
        #define Load_necessary(BaseAddr) asm volatile
        //保存和恢复当前寄存器信息的定义
        //Op: ld x | sd x | fld f | fsd f|
        #define Context_Operation(Op, BaseAddr) asm volatile
    ```
    - 全局函数和变量的定义
    ```c
        //takeoverSyscall函数的入口地址（并不是函数的第一条指令）
        extern uint64_t takeOverAddr;
        //代码段的信息，代码段的所有区域都会被映射到内存中
        extern MemRangeInfo text_seg;           
        //接管系统调用并处理的函数
        void takeoverSyscall(); 
        //加载elf文件的代码段
        uint64_t loadelf(char * progname, char *ckptinfo);
        //读取ckpt文件中的信息，并恢复benchmark的执行环境
        void read_ckptinfo(char ckptinfo[]);
    ```

2. readckpt.cpp: main函数入口，主要负责初始分配一些固定的内存区域
    - main(int argc, char **argv)
      - argv[1]: ckpt文件名; argv[2]: benchmark的elf文件
      - 以RunningInfoAddr为基址，分配8KB空间，用于存储结构体RunningInfo
        - 固定地址的目的在于可以直接使用固定地址访问到，不会受限于当前处于启动程序还是测试程序中
      - 调用loadelf(argv[2], argv[1])
      - 调用read_ckptinfo(argv[1])'

3. elf_load.cpp: 分配benchmark的代码段，并且加载代码到内存中
    - uint64_t loadelf(char * progname, char *ckptinfo)
      - 解析ELF文件，获取代码段的信息
      - 读取ckpt文件，获取使用的代码区域信息，并从elf文件中将这些信息加载到内存中
  
4. ckpt_load.cpp: 完成对ckpt的读取，对benchmark的环境恢复
    - void read_ckptsyscall(FILE *fp)
      - 从ckpt文件中读取所有系统调用的信息放到内存中
      - 将所在的内存地址记录在runinfo->syscall_info_addr中，用于接管系统调用时处理
      - 设置runinfo->nowcallnum和runinfo->totalcallnum
      - 遍历所有的系统调用信息，根据系统调用ecall指令的pc，将ecall指令替换为jmp Rtemp1指令
    - void alloc_memrange(FILE *p)
      - 从ckpt文件中读取所有的内存区域信息MemRangeInfo
      - 将所有的内存区域使用map函数进行映射
      - 由于之前代码段已经完成了映射，因此判断当前内存区域是否和代码段有部分重叠，如果有则需要单独处理和映射
      - （访问执行过程中访问了代码段的部分区域，如果不映射代码段所有区域，那就需要单独比较所有记录的代码区域）
    - void setFistLoad(FILE *p)
      - 从ckpt文件中读取所有的first load信息，并且将其恢复到内存中
    - void read_ckptinfo(char ckptinfo[])
      - 主要任务：读取ckpt文件，恢复所有执行环境，完成跳转转换，启动benchmark执行
      - 读取ckpt文件的头部，获取程序片段的起始指令数，片段指令数（到退出指令为止），设置的片段长度，warmup的指令数，退出指令的pc，退出的原因，片段的第一条指令pc地址
      - 从ckpt文件中获取整数和浮点寄存器的数据，存储到runinfo->intregs和fpregs中
      - 调用alloc_memrange，分配所有内存区域
      - 调用setFirstload，恢复所有first load的数据
      - 调用read_ckptsyscall，读取所有的系统调用信息
      - 调用getRangeInfo和produceJmpInst，完成对所有跳转需求的转换
      - 判断runinfo->totalcallnum == 0，如果是，则此时就需要将退出指令替换为jmp Rtemp1指令；否则交由takeOverSyscall函数在最后一次系统调用处理完后进行替换
      - 将npc保存到临时寄存器0中，用于跳转使用：WriteTemp(0, npc)
      - 将takeOverAddr保存到临时寄存器1中，即jmp Rtemp1指令将会跳转到takeOverAddr，完成ecall到takeOverSyscall函数的跳转：WriteTemp(1, takeOverAddr);
      - 将加载器的寄存器状态保存到OldIntRegAddr中（runninginfo->oldregs）
      - 根据ckpt中记录的warmup和原本片段长度信息，设置最大指令数和预热指令数；如果ckpt没有该信息，则预热指令数为simNum的5%，最大指令数为剩余的指令数
      - 恢复ckpt起始位置处的寄存器状态
      - 利用跳转到临时寄存器的指令JmpTemp(0)，完成跳转到npc的工作

5. takeover_syscall.cpp: 接管ecall，完成对内存的更改和返回值的设定
    - takeoverSyscall函数：用于接管系统调用，完成对内存的修改，设定返回值，监测运行是否正确，完成退出指令的替换
    - 工作流程：
      - 以a0为基址寄存器 = StoreIntRegAddr，保存当前所有整数寄存器（应该不会修改浮点寄存器，所以不保存）
      - 恢复加载器的一些必要寄存器：sp, fp, gp, tp
      - 根据runinfo->syscall_info_addr, 读取系统调用的信息到SyscallInfo *infos中
      - 根据系统调用信息中的p0（a0寄存器），恢复最开始没有保存的a0寄存器
      - 判断runinfo->nowcallnum >= runinfo->totalcallnum，如果超过了，表示已经执行结束了
        - exitinst也会跳转到该函数，因此这个时候nowcall会大于totalcallnum，所以表示退出
      - 判断runinfo->intregs[17] (a7) != infos->num，判断系统调用号是否和记录的一致，如果不一致，则表示运行错误
      - 判断infos->data_offset != 0xffffffff，如果不是，则表示该系统调用需要修改内存数据，此时根据记录的数据和bufaddr信息修改内存即可
      - 判断infos->hasret是否有效，如果有效，则根据infos->ret修改返回寄存器a0
      - runinfo->nowcallnum++
      - 判断runinfo->nowcallnum == runinfo->totalcallnum是否已经是最后一次系统调用，如果是，则将退出指令所在的地址替换为jmp RTemp1指令
        - *((uint32_t *)runinfo->exitpc) = ECall_Replace
      - 将系统调用的返回地址写入到临时寄存器0中
        - WriteTemp(0, npc)
      - 完成系统调用的处理，恢复benchmark的寄存器状态
      - 使用JmpTemp(0)跳转到系统调用的下一条指令，继续执行