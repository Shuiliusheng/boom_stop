### create ckpt with gem5
1. 执行和支持的log信息类型
    - 参数：--debug-flag=ShowMemInfo,ShowRegInfo,ShowSyscall 
        - 利用debug-flag来确定要输出的信息
        - ShowMemInfo用于控制输出访存的信息: load, store, atmoic
            ```json
                {"type": "mem_read", "pc": "0x20b00e", "addr": "0x25da40", "size": "0x8", "data": "0x25c218"}
                {"type": "mem_write", "pc": "0x20b014", "addr": "0x7ffffbd8", "size": "0x8", "data": "0x200d82"}
                {"type": "mem_atomic", "pc": "0x20b0b2", "addr": "0x25ec60", "size": "0x4", "data": "0x1"}
            ```
        - ShowSyscall用于控制输出系统调用信息
            ```json
                {"type":"syscall enter", "pc": "0x2171f6", "sysnum": "0x40", "param": [ "0x1", "0x2612e0", "0x3d", "0x2612e0", "0x2a0" ]}
                {"type":"syscall info", "info": "write", "pc": "0x2171f6", "fd": "0x1", "buf": "0x2612e0", "bytes": "0x3d", "ret": "0x3d", "data": [ "0x34","0x30","0x20","0x34","0x31","0x20"]}
                {"type":"syscall return", "sysnum": "0x40", "sysname":"(write(61, 2495200, 61))", "pc": "0x2171f6", "res":"has ret", "val": "0x3d"}

                {"type":"syscall enter", "pc": "0x216d34", "sysnum": "0xa9", "param": [ "0x7ffffc08", "0x0", "0xf", "0x1", "0x1" ]}
                {"type":"syscall info", "info": "setdata", "pc": "0x216d34", "buf": "0x7ffffc08", "bytes": "0x10", "ret": "0x0", "data": [ "0x0","0xca"]}
                {"type":"syscall return", "sysnum": "0xa9", "sysname":"(gettimeofday(0, 0))", "pc": "0x216d34", "res":"has ret", "val": "0x0"}
            ```
        - ShowRegInfo用于控制输出startpoint的信息和一些额外信息
            - isSyscall记录系统调用发生时的模拟指令数
            - ckptExitInst记录exitInst的一些信息
            - textRange记录程序模拟过程中访问的代码段的区域信息
            ```json
                {"type": "int_regs", "inst_num": "80000", "inst_pc": "0x200708", "npc": "0x20070c", "data": [ "0x0", "0x2006fc", "0x7ffffbf0", "0x25e1a8", "0x260710", "0x0", "0x25d6b8", "0x80000", "0x7ffffdf0", "0x200d82", "0x263520", "0x1f40", "0x25c6b8", "0x7ffffde0", "0x1b5", "0x7cf", "0x0", "0x1000", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x1f3f", "0x78", "0x23d2b6", "0x1" ]}
                {"type": "fp_regs", "inst_num": "80000", "inst_pc": "0x200708", "npc": "0x20070c", "data": [ "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0", "0x0" ]}

                {"type": "isSyscall", "inst_num": "112191", "pc": "0x2171f6"} 
                {"type": "ckptExitInst", "inst_num": "28993", "inst_pc": "0x200642"}
                {"type": "textRange", "addr": [ "0x200000", "0x204000", "0x207000", "0x220000", "0x229000", "0x22b000", "0x230000", "0x23c000" ], "size": [ "0x1000", "0x2000", "0x13000", "0x2000", "0x1000", "0x4000", "0x2000", "0x1000" ] }
            ```
    - 参数：--debug-file=logfile , 用于控制log文件的位置和名称，原本gem5的参数
    - 参数：--ckptinsts=4000 
        - 用于控制间隔多少条指令开始创建checkpoint
        - 主要相关是控制是否输出 int_regs和fp_regs 的信息，其他均不影响
    - 参数：--startinsts, --endinsts
        - startinsts用于控制从多少条指令开始记录ckpt的信息
        - endinsts用于控制多少条指令开始停止记录ckpt的信息
        - [startinsts, endinsts]会按ckptinsts为间隔创建ckpt的startinfo
        - 如果没有设置，则从开始到执行结束或者执行到maxinsts处为止，统计ckpt
    - 参数：--stackbase, --mmapend
        - stackbase用于设置模拟过程中设置的stacktop的地址
        - mmapend用于设置mmap可分配的最大上限
        - 未设置，则采用sim/Process.py中的默认设置
            - stackbase = Param.UInt64(0x7FFFFFFF, 'simulation stack base of program')
            - mmapend = Param.UInt64(0x41000000, 'simulation the mmap addr end place')
        - 目前riscv-pk采用的配置：--stackbase=801017856 (0x2fbe9000)  --mmapend=789483520 (0x2f0e9000)

2. 运行示例
    ```shell
        build/RISCV/gem5.opt --debug-flag=ShowMemInfo,ShowRegInfo,ShowSyscall --debug-file=big1.log ./configs/example/se.py --startinsts=5000000 --endinsts=11000000 --stackbase=801017856 --mmapend=789483520 --ckptinsts=2000000 -c ckptinfo/test/big1.riscv
    ```

3. log的预处理
    - 在gem5的执行过程中，会对之后不会使用到的load/store/atomic的信息进行初步的筛查，以减少log文件的大小
    - 将log信息中的一些无用信息剔除，仅包括{}中的json信息，用于之后的识别
    - 将textRange信息从文件最后移到文件最前，方便之后创建ckpt
    ```shell
        #!/bin/bash
        #ckptinfo/preproc.sh
        if [[ $# < 1 ]]; then
            echo "parameter is not enought!"
            exit
        fi

        textinfo=`grep -r textRange $1 | awk -F '{' '{print "{"$2}' `
        echo $textinfo
        echo $textinfo >temp.log
        cat $1 | awk -F '{' '{print "{"$2}' >> temp.log
        mv temp.log $1
    ```