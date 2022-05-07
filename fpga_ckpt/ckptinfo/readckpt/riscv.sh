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
temp=`grep -r "<_Z15takeoverSyscallv>:" read.s -A 10 |grep "zero,a0,sp"`
echo $temp
grep -r "#define TakeOverAddr" info.h

entry=`echo $temp|awk -F ':' '{print $1}' |awk -F ' ' '{print $1}'`
sed -i "s/#define TakeOverAddr.*/#define TakeOverAddr 0x$entry/g" info.h

#recompile
${cc} $filelist $flags $dflags -o ${target}
${objdump} -d ${target} >read.s



