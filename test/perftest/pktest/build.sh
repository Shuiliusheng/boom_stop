#!/bin/bash

#判断必要的头文件是否存在
if [[ ! -e "ctrl.h" ]]; then
    echo "ctrl.h is not exit!"
    exit
fi

if [[ ! -e "define.h" ]]; then
    echo "define.h is not exit!"
    exit
fi

# 第一次编译文件
riscv64-unknown-linux-gnu-gcc pftest.c -static -o pftest.riscv


# 获取exit_fuc的入口地址
entry="zero,a0,4"
riscv64-linux-gnu-objdump -d pftest.riscv >temp.s
line=`grep -r exit_fuc temp.s -A 10 | grep "$entry"`
pc=`echo $line | awk -F ':' '{print $1}' | awk -F ' ' '{print $1}'`
# 替换ctrl.h中关于入口地址的设定
sed -i "s/unsigned long long exitFucAddr.*/unsigned long long exitFucAddr=0x$pc;/g" ctrl.h


# 重新编译
echo $line
echo "entrypoint:" "0x"$pc
riscv64-unknown-linux-gnu-gcc pftest.c -static -o pftest.riscv


