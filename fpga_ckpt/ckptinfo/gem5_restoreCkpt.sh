#!/bin/bash

if [[ $# < 2 ]]; then
    echo "parameters are not enough!"
    echo $1 $2
    exit
fi

#info directory
infodir="ckptinfo/info/mcfinfo"
readckpt="ckptinfo/readckpt/readckpt.riscv"

bench=$1
simInsts=$2

basename=`echo $(basename $bench) |awk -F '.' '{print $1}'`
ckptinfo=$infodir/${basename}_ckpt_${simInsts}.info
ckptsysinfo=$infodir/${basename}_ckpt_syscall_${simInsts}.info

option="--options=$ckptinfo $ckptsysinfo $bench"

debugflags="" #--debug-flag=ShowDetail

if [[ ! -e $bench ]]; then
    echo $bench " is not exist!"
    exit
fi 

if [[ ! -e $ckptinfo ]]; then
    echo $ckptinfo " is not exist!"
    exit
fi 

if [[ ! -e $ckptsysinfo ]]; then
    echo $ckptsysinfo " is not exist!"
    exit
fi 

if [[ ! -e $readckpt ]]; then
    echo $readckpt " is not exist!"
    exit
fi 

echo "build/RISCV/gem5.opt $debugflags ./configs/example/se.py -c $readckpt "$option""
param="--cpu-type=DerivO3CPU --caches --mem-size=8GB --l1d_size=64kB --l1d_assoc=8 --l1i_size=16kB --l1i_assoc=4 --l2cache --l2_size=2MB --l2_assoc=16"
fast="--fast-forward=38000000"
build/RISCV/gem5.opt $debugflags ./configs/example/se.py -c $readckpt "$option"
