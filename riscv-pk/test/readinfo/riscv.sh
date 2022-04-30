#!/bin/bash

cc=riscv64-unknown-linux-gnu-gcc
objdump=riscv64-unknown-linux-gnu-objdump
flags="-static -O2"
dflags=""
target="readinfo.riscv"
if [[ $# > 0 ]]; then
    dflags="-DSHOWLOG"
fi

filelist="takeover_syscall.cpp ckpt_load.cpp elf_load.cpp readinfo.cpp"
echo ${cc} $filelist $flags $dflags -o ${target}
${cc} $filelist $flags $dflags -o ${target}

echo ${objdump} -d ${target} 
${objdump} -d ${target} >read.s
