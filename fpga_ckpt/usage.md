### 创建checkpoint
1. 使用新的链接脚本编译源文件，将代码段和数据段的起始地址设置到0x20000的位置
    - 链接脚本的位置：ckptinfo/test/link.lds
    - 示例：
    ```shell
    riscv64-unknown-linux-gnu-gcc test.c -c -o test.o
    riscv64-unknown-linux-gnu-gcc test.o -static -T link.lds -o test.riscv
    ```

2. 使用podman_create.sh脚本启动gem5，使用ckptinfo/gem5_createCkpt.sh脚本执行编译好的程序，创建ckpt
    - ckptinfo/gem5_createCkpt.sh中的一些设置
        - 输入参数：./gem5_createCkpt.sh 可执行程序 Ckpt的间隔指令数 "可执行程序的参数"
        - stacktop & mmapend：目前是根据pk来设置的，将运行时的栈顶设置在一个固定位置，避免恢复时和加载器的栈冲突
        - maxinsts： gem5运行的最大指令数
        - startinsts： 从多少条指令开始创建checkpoint
        - endinsts: 从多少条指令开始停止创建ckpt
        - 生成的log文件：m5out/*.log

3. 将生成的log文件，m5out/*.log，移动到ckptinfo/info目录，使用preproc.sh脚本处理
    - preproc.sh的输入：./preproc.sh *.log ckpt保存的目录
    - preproc会先预处理log文件，然后调用ckptinfo/createckpt/process.py脚本生成ckpt文件


### 恢复checkpoint
1. 使用ckptinfo/readckpt目录中编译得到的加载器执行即可
    - 执行参数：./readckpt.riscv bench_ckpt_num.log bench_ckpt_syscall_num.log bench
        - bench_ckpt_num.log bench_ckpt_syscall_num.log为ckpt的两个文件
        - bench为原本的测试程序二进制
        - 不再需要测试程序的任何输入和输入文件