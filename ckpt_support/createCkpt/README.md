#### create ckpt with gem5
#### 使用方法
1. 创建ckpt示例
    - 额外注意：编译程序时需要指定特定的link文件，具体示例在bench中存在(build.sh)
    ```shell
        ./build/RISCV/gem5.opt --debug-flag=CreateCkpt --debug-file=temp ./configs/example/se.py --mem-type=SimpleMemory --mem-size=8GB --ckptsetting=settings -c bench/test.riscv
    ```
2. gem5之外的参数解释
    - (除此之外均为gem5原本的参数)
    - 参数：--debug-flag=CreateCkpt
        - 用于表示是否需要创建checkpoint
    - 参数：--debug-file=logfile (可以不需要)
        - 用于控制log文件的位置和名称，原本gem5的参数
    - 参数：--ckptsettings=settingsfile 
        - 用于指定生成ckpt的一些参数信息，包括
            - mmapend/stacktop: 指定运行时的栈基址和map的最大位置
            - ckptprefix: 指定输出ckpt的目录和ckpt文件名前缀
            - ckptctrl: (startckpt interval warmupnum times), 文件中可以存在多个
                - startckpt: 从多少条指令开始创建ckpt
                - interval: 从startckpt开始间隔多少条指令创建一个ckpt
                - warmupnum: 每一个ckpt提前准备多少条指令用于恢复时的预热
                - times: 创建几个ckpt
        ```c
            mmapend: 261993005056
            stacktop: 270582939648
            ckptprefix: ./test
            ckptctrl: 10000 10000 1000 4
            ckptctrl: 60000 20000 1000 4
        ```

#### 工作方式简介
1. 主要工作内容
    - 根据设置的ckpt起始指令和终止指令的信息，统计
        - 该间隔开始时寄存器的信息
        - 该间隔内的代码段访问信息
        - 该间隔内的内存区域访问信息
        - 该间隔内的第一次load的指令信息
    - 在执行过程中寻找能够结束该ckpt片段的特殊的退出指令
        - 该指令在ckpt片段内的最后一次系统调用后第一次出现
    
2. 主要步骤
    - 在执行到指定的指令数时，创建ckpt对象开始记录(class CkptInfo), 记录此时的寄存器状态
    - 记录到片段结束之前执行的指令地址，最后确定代码段区域 (set<uint64_t> textAccess)
    - 按顺序记录load/store/atomic三种存储操作的地址，最后用于确定访问的内存区域 (set<uint64_t> preAccess)
    - 遇到load指令时，判断该指令访问的地址是否在之前出现过(preAccess)，如果没有出现，则为first load
        - 需要注意：load指令有数据宽度，因此统计时将其拆分为单位为1byte的多条load进行判断
    - 记录到片段结束之前执行的系统调用的信息：系统调用指令地址，参数，系统调用号，修改的内存地址和数据，返回值
    - 寻找退出指令，结束ckpt的创建，根据记录的信息生成ckpt文件
    - 需要注意：
        - 由于退出指令的存在，使得记录的ckpt片段指令数会超过预定的指令数
        - 由于退出指令的存在，执行过程中同一个时间节点，可能会存在多个ckpt在创建


3. 创建ckpt文件 (saveCkptInfo)
    - 将textAccess和preAccess转换为区域信息，以4KB为基础单位，将其分别划分为多个区域(textrange, memrange)
    - 将记录的以1B为单位的first load进行合并，将其合并为以8B为单位的load指令（没有则为0）
    - 将ckpt基础信息和textrange, memrange, firstload(所有load的大小都为8B), syscallinfo写入文件中
    - 文件格式: prefix_ckpt_{startplace}.info
        | textinfo num              | 8      |
        |:-------------------------:|:------:|
        | textinfos(addr, size)     | num*16 |
        | sim start place           | 8      |
        | sim inst num & warmupnum | 8      |
        | exit inst pc              | 8      |
        | exit cause & ckptlength   | 8      |
        | start npc                 | 8      |
        | start int regs            | 8*32   |
        | start ip regs             | 8*32   |
        | memrange num              | 8      |
        | memrange info(addr, size) | num*16 |
        | first load num            | 8      |
        | load (addr, data)         | num*16 |
        | **syscall info num**      |**8**   |
        | pc                        | 8      |
        | num                       | 8      |
        | p0                        | 8      |
        | p1                        | 8      |
        | p2                        | 8      |
        | hasret                    | 8      |
        | ret                       | 8      |
        | bufaddr                   | 8      |
        | data_offset (file offset) | 8      |
        | data size                 | 8      |
        |**all syscall data information**    |

4. 创建ckpt简介信息的文件 (saveDetailInfo)
    - 文件名: prefix_infor_{startplace}.txt
    - 内容：
        - ckptinfo: startnum, exitnum, length, warmup, pc, npc, exitpc
        - text range num, mem range num, first load num, syscallnum
        - integer register value
        - float register value
        - text range information
        - mem range information
        - running instruction information


5. 代码修改
    - 增加的文件: gem5_create/sim/ckpt_collect.cc  ckpt_collect.hh  ckptinfo.cc  ckptinfo.hh
    - 修改的文件:
        - ckpt配置: gem5_create/sim/process.cc SConscript arch/riscv/process.cc
        - 基本统计: gem5_create/cpu/simple/atomic.cc atomic.hh
        - 系统调用: gem5_create/sim/syscall_emul.hh sim/syscall_desc.cc arch/riscv/linux/se_workload.cc