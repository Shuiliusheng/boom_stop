#!/bin/bash


if [[ $# < 1 ]]; then
    echo "parameters are not enough!"
    exit
fi

echo "build $1"
name=`ls $1 | awk -F '.' '{print $1}'`

riscv64-unknown-linux-gnu-gcc $1 -c -o $name.o
riscv64-unknown-linux-gnu-gcc $name.o -static -T link.lds -o $name.riscv
rm $name.o


#build/RISCV/gem5.opt --debug-flag=ShowMemInfo,ShowRegInfo,ShowSyscall  ./configs/example/se.py --ckptinsts=5000 -c ckptinfo/test/main
