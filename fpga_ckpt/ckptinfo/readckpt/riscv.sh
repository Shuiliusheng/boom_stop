#!/bin/bash

cc=riscv64-unknown-linux-gnu-gcc
objdump=riscv64-unknown-linux-gnu-objdump
flags="-static -O2"
dflags=""
target="readckpt.riscv"
if [[ $# > 0 ]]; then
    dflags="-DSHOWLOG"
fi

filelist=`find . -name "*.cpp" `
echo ${cc} $filelist $flags $dflags -o ${target}
${cc} $filelist $flags $dflags -o ${target}

echo ${objdump} -d ${target} 
${objdump} -d ${target} >read.s

# find the takeOversyscall entry point place
echo "replace TakeOverAddr in info.h with new value"
temp=`grep -r "<_Z15takeoverSyscallv>:" read.s -A 10 |grep "00a56013"`
entry=`echo $temp|awk -F ':' '{print $1}' |awk -F ' ' '{print $1}'`
if [[ "$entry" == "" ]]; then
    echo "cannot find TakeOverAddr!"
    exit
fi
sed -i "s/#define TakeOverAddr.*/#define TakeOverAddr 0x$entry/g" info.h
echo $temp
grep -r "#define TakeOverAddr" info.h


# 获取exit_fuc的入口地址
echo ""
echo "replace exitFucAddr in ctrl.h with new value"
exit_fuc_tag="zero,a0,4"
line=`grep -r exit_fuc read.s -A 10 | grep "$exit_fuc_tag"`
exit_fuc_pc=`echo $line | awk -F ':' '{print $1}' | awk -F ' ' '{print $1}'`
if [[ "$exit_fuc_pc" == "" ]]; then
    echo "cannot find exitFucAddr!"
    exit
fi
# 替换ctrl.h中关于入口地址的设定
sed -i "s/unsigned long long exitFucAddr.*/unsigned long long exitFucAddr=0x$exit_fuc_pc;/g" ctrl.h
echo $line
grep -r "unsigned long long exitFucAddr"  ctrl.h

#recompile
${cc} $filelist $flags $dflags -o ${target}
${objdump} -d ${target} >read.s
