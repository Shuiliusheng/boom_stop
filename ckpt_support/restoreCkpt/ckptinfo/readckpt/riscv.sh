#!/bin/bash

cc=riscv64-unknown-linux-gnu-gcc
objdump=riscv64-unknown-linux-gnu-objdump
flags="-static -O2"
dflags=""
target="readckpt.riscv"

filelist=`find . -name "*.cpp" `
echo ${cc} $filelist $flags $dflags -o ${target}
${cc} $filelist $flags $dflags -o ${target}

echo ${objdump} -d ${target} 
${objdump} -d ${target} >read.s