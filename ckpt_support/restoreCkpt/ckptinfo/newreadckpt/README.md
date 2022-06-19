### 读取生成的ckpt文件，加载测试程序运行（不需要硬件支持）
1. ckptinfo.h: 唯一的头文件，用于定义一些struct, 汇编代码, 函数, extern变量
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
        #define TPoint1 0x100000
        #define TPoint2 0x1FF000
        //riscv中jal最大的跳转距离
        #define MaxJALOffset 0xFFFF0
    ```
    - 结构体定义
    ```c
        typedef struct{
            uint64_t addr;
            uint32_t inst;
        }JmpInfo;  //记录跳转指令和跳转指令所在的地址

        typedef struct{
            int num;
            JmpInfo *infos;
        }JmpRepInfo; //记录跳转到某一个地址时所需要的跳转指令的数量和具体信息

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
            uint32_t exitJmpInst;   //退出指令要被替换为的跳转指令
            JmpRepInfo *sysJmpinfos;//完成系统调用返回所需要的跳转指令系统
        }RunningInfo; 
        //ckpt恢复执行后需要的一些信息
        //设置在固定的地址RunningInfoAddr，从而能够找到这些信息
    ```
    - 内联汇编定义
    ```c
        //加载器向benchmark执行环境跳转的第一步，跳转到TPoint1的中间节点
        #define StartJump() asm volatile
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
        //在instaddr地址处设置jal指令，使其可以跳转到target地址处
        bool setJmp(uint64_t instaddr, uint64_t target);
        //寻找满足所有跳转需求的中间节点
        void initMidJmpPlace();
        //获取ckpt中所有的内存区域信息
        void getRangeInfo(char filename[]);
        //根据中间节点和跳转需求，确定所有跳转指令
        void produceJmpInst(uint64_t npc);
        //根据JmpRepInfo，完成对跳转指令的设置
        void updateJmpInst(JmpRepInfo &info);
    ```

2. readckpt.cpp: main函数入口，主要负责初始分配一些固定的内存区域
    - main(int argc, char **argv)
      - argv[1]: ckpt文件名; argv[2]: benchmark的elf文件
      - 调用initMidJmpPlace
      - 以RunningInfoAddr为基址，分配8KB空间，用于存储结构体RunningInfo
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
      - 判断runinfo->totalcallnum == 0，如果是，则此时就需要将退出指令替换为jal指令；否则交由takeOverSyscall函数在最后一次系统调用处理完后进行替换
      - 设置跳转到npc的跳转指令序列
        - updateJmpInst(runinfo->sysJmpinfos[0])
      - 将加载器的寄存器状态保存到OldIntRegAddr中（runninginfo->oldregs）
      - 恢复ckpt起点处的寄存器状态
      - 使用StartJump开始跳转到npc开始执行

5. user_jmp.cpp: 完成寻找能够满足所有跳转需求的地址的跳转指令
    - vector<MemRangeInfo> freeRanges;
      - 用于记录所有空闲内存区域的信息
    - vector<uint64_t> midpoints;
      - 记录能够覆盖所有需要跳转的地方的中间点地址
    - initMidJmpPlace()
      - 分配TPoint1和TPoint2所在的页面
      - 这两个点为必要的点，因为需要加载器首先就需要跳转到0x200000(benchmark代码段的起始地址)
      - 如果这两个页面无法初始化，则退出
    - uint32_t createJAL(uint64_t base, uint64_t target)
      - 根据基址base和目标地址target创建riscv的jal指令，返回是32位的指令
    - bool setJmp(uint64_t instaddr, uint64_t target)
      - 在instaddr地址处设置jal指令，使其跳转到target处
    - void getRangeInfo(char filename[])
      - 读取ckpt文件，从中获取所有benchmark需要用的内存区域，包括代码段和数据段
      - 根据这些区域，找到代码段中所有尚未分配的内存区域，用于之后寻找中间点
    - void findMidPoints(uint64_t base, uint64_t maxtarget)
      - 根据最初的地址TPOINT2和所有需要的跳转中最远的目标地址，在freeRanges中寻找满足的中间点
      - 所有中间点会记录在midpoints中
    - void replaceSyscall(vector<uint64_t> &pcs) 
      - 将所有系统调用指令替换为jal指令，使其能够沿着midpoint跳转到takeOverAddr指向的地方
      - pcs中存储着所有系统调用的pc
      - 在该函数中也会确定exitpc应该放置的跳转指令
      - 所有的系统调用的跳转路径的指令将会在此时直接写入到对应的内存地址
      - 示例：ecallpc -> (midpoint2 + 4) -> (midpoint1 + 4) -> TPOINT2 + 4 -> TPOINT1 + 4 -> takeOverAddr
    - void produceSysRet(vector<uint64_t> &pcs)
      - 寻找跳转到benchmark第一条指令地址和系统调用返回地址的跳转指令序列
      - 方法类似于replaceSyscall，但是区别在于此时仅记录该路径的信息而不直接设置指令，因此每个跳转需要设置的内容并不相同，只能够在遇到时设置，并立马使用
      - 每个系统调用返回地址的跳转路径信息记录在runinfo->sysJmpinfos中，在处理完系统调用后完成设置和跳转
    - void produceJmpInst(uint64_t npc)
      - 对外的接口函数
      - 汇总所有需要跳转的目标地址到pcs变量中 -> findMidPoints -> replaceSyscall -> produceSysRet -> setJmp(TPoint1, TPoint2)
    - void updateJmpInst(JmpRepInfo &info)
      - 根据JmpRepInfo结构体中记录的跳转路径信息，将对应的跳转指令写入到对应的地址中
      - 主要用于设置系统调用返回地址和benchmark第一条指令地址


6. takeover_syscall.cpp: 接管ecall，完成对内存的更改和返回值的设定
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
      - 判断runinfo->nowcallnum == runinfo->totalcallnum是否已经是最后一次系统调用，如果是，则将退出指令所在的地址替换为跳转指令
        - *((uint32_t *)runinfo->exitpc) = runinfo->exitJmpInst
      - 更新返回到该系统调用下一条指令所需要的跳转指令
        - updateJmpInst(runinfo->sysJmpinfos[runinfo->nowcallnum])
      - 完成系统调用的处理，恢复benchmark的寄存器状态
      - startJump开始跳转回到系统调用的返回地址