### 处理gem5生成的运行时log文件，生成ckpt

1. 主要工作
    - 确定[start, exitInsts]中的start处的起始信息，访存信息，系统调用信息，使用的代码区域信息
    - 确定[start, exitInsts]的区域
    - 整理起始点信息
    - 整理系统调用信息
    - 整理访存信息，获取first load的信息
    - 根据访存信息和系统调用信息，确定[start, exitInsts]过程中使用的内存区域范围
    - 创建ckpt文件

2. 确定[start, exitInsts]的区域
    - 逐行读取文件，根据type信息，将不同信息收集起来
        - reginfo, meminfo, syscallinfo, textinfo, syscallplace
    - 当第二次遇到type == int_regs，处理上一阶段的信息，此时能够知道上一阶段的start信息
    - 获取当前文件位置，在寻找到exitInst之后，重新回到该位置，避免将太多的数据加载到内存中
    - 从当前位置开始继续向后寻找，直到找到第一次 type == isSyscall or ckptExitInst，此时确定该位置出的simNum为exitInst的位置
    - 将[start, exitInsts]中的所有访存信息都记录下来，用于之后处理
    ```python
        class CkptExitInfo:
            exitpc = 0
            inst_num = 0
            reason = ""
            last_syspc = 0
            last_sysnum = 0

            def show(self):
                print("last syscall info", 'instnum: %d'%self.last_sysnum, 'pc: 0x%x'%self.last_syspc)
                print("choiced exit info", 'instnum: %d'%self.inst_num, 'pc: 0x%x'%self.exitpc, self.reason)
    ```

3. 整理起始点信息: 根据type == int_regs中的信息获取以下信息
    - 记录start处的模拟指令数
    - 记录此时指令的pc和下一条指令的npc
    - 记录整数和浮点数寄存器的内容
    ```python
        class StartPoint:
            inst_num = ""
            pc = " "
            npc = " "
            intdata = []
            fpdata = []
    ```
    ```json
        {"type": "int_regs", "inst_num": "60000", "inst_pc": "0x208006", "npc": "0x20800a", "data": [ "0x0", "0x2052b8", "0x7ffff5f0", "0x25e1a8", "0x260710", "0xa", "0x7fffffde", "0x2", "0x7ffffdf0", "0x200d82", "0x3", "0x23d263", "0x20", "0x1", "0x1", "0x23d263", "0x64", "0xfffffffffffffffe", "0x0", "0x0", "0x0", "0x0", "0x0", "0x25c218", "0x20", "0x23dbc0", "0x3", "0x1", "0x0", "0x59", "0x0", "0x20" ]}
    ```

4. 整理系统调用信息: 根据 type == syscall* 的line中收集以下信息
    - 系统调用信息必须包含的两个信息
        - type == syscall enter 
        - type == syscall return
        ```json
            {"type":"syscall enter", "pc": "0x2171f6", "sysnum": "0x40", "param": [ "0x1", "0x2612e0", "0x20", "0x2612e0", "0x2a0" ]}
            {"type":"syscall return", "sysnum": "0x40", "sysname":"(write(32, 2495200, 32))", "pc": "0x2171f6", "res":"has ret", "val": "0x20"}
        ```
    - 系统调用发生用户内存数据修改时的信息 
        - type == syscall info, 用于记录要写的地址和数据
        - 通常info字段为setdata, 但是单独处理了read/write/open
        ```json
            {"type":"syscall info", "info": "write", "pc": "0x2171f6", "fd": "0x1", "buf": "0x2612e0", "bytes": "0x20", "ret": "0x20", "data": [ "0x73","0x74","0x61","0x72","0x74","0x20","0x73","0x65","0x63","0x3a","0x20","0x31","0x30","0x30","0x30","0x30","0x30","0x30","0x30","0x30","0x30","0x2c","0x20","0x75","0x73","0x65","0x63","0x3a","0x20","0x34","0x33","0xa" ]}

            {"type":"syscall info", "info": "setdata", "pc": "0x217064", "buf": "0x7ffffa90", "bytes": "0x80", "ret": "0x0", "data": [ "0x53","0x0","0x0"]}
        ```
    - 当前每个系统调用所统计的信息
    ```python
        class SyscallInfo:
            pc = ""
            num = ""
            name = ""
            params = []
            hasret = False
            ret = ""
            data = []
            bufaddr = "" #用于记录要写内存的地址
    ```

5. 整理访存信息，获取first load的信息
    - first load：当前load指令所访问的内存数据之前没有其他store写过，也没有被之前的load指令加载过
    - 使用两个set来寻找first load的信息
        - set prestore: 记录之前被store的地址
        - set preload: 记录之前被load的地址
        - 从开始遍历访存序列，将访存地址按照访存大小进行展开，使得访存单位变为byte
        - 如果load的某个地址不在prestore和preload中，则为first load，需要记录到loadinfo中
        - 将地址信息分别记录prestore和preload中
    - 将loadinfo中的信息，按照地址顺序排序，然后进行合并，使得load的数据单位从8->4->2->1的优先级进行组合
    ```python
        class Memdata:
            addr = 0
            data = 0
            size = 0
    ```

6. 根据访存信息和系统调用信息，确定[start, exitInsts]过程中使用的内存区域范围
    - 根据meminfo中的信息，将所有load和store的地址按照访存大小进行展开，使得访存单位变为byte，然后将这些地址放入addrset集合中
    - 根据syscallinfos中的信息，将系统调用所访问的内存区域的地址放入addrset中
    - 对addrset进行排序，遍历，以4KB为单位，进行第一次合并，得到以4KB对齐的起始地址+4KB的区域
    - 将相邻的4KB区域进行合并，得到最终的memrange信息


7. 创建ckpt文件
    - 将textinfo, exitinfo, startinfo, memrangeinfo, loadinfo, syscallinfo写入文件中，得到ckpt文件
    - 文件1: ckpt_{startplace}.info，记录textinfo, exitinfo, startinfo, memrangeinfo, loadinfo
        --------------------------------------
        | textinfo num              | 8      |
        --------------------------------------
        | textinfos(addr, size)     | num*16 |
        --------------------------------------
        | sim start place           | 8      |
        --------------------------------------
        | sim inst num              | 8      |
        --------------------------------------
        | exit inst pc              | 8      |
        --------------------------------------
        | start npc                 | 8      |
        --------------------------------------
        | start int regs            | 8*32   |
        --------------------------------------
        | start ip regs             | 8*32   |
        --------------------------------------
        | memrange num              | 8      |
        --------------------------------------
        | memrange info(addr, size) | num*16 |
        --------------------------------------
        | first load num            | 8      |
        --------------------------------------
        | load (addr, size, data)   | num*24 |
        --------------------------------------

    - 文件2: ckpt_syscall_{startplace}.info，记录syscallinfo
        --------------------------------------
        | syscall info num          | 8      |
        --------------------------------------
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
        --------------------------------------
        | all syscall data information       |
        --------------------------------------