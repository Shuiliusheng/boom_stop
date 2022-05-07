#!/bin/bash

if [[ $# < 2 ]]; then
    echo "parameters are not enough!"
    exit
fi

bench=$1
if [[ ! -e $bench ]]; then
    echo $bench " is not exist!"
    exit
fi 

ckptinsts=$2
options="--options=$3"

basename=`echo $(basename $bench) |awk -F '.' '{print $1}'`
logfile="$basename.log"

#riscv-pk stack = 3fbe9000, mmap = 3fbe9000
stacktop=801017856  #0x2fbe9000
mmapend=789483520   #0x2f0e9000

maxinsts=500000000
startinsts=200000000
endinsts=250000000

debugflags="ShowMemInfo,ShowRegInfo,ShowSyscall"


echo "build/RISCV/gem5.opt --debug-flag=$debugflags --debug-file=$logfile ./configs/example/se.py --stackbase=$stacktop --mmapend=$mmapend --ckptinsts=$ckptinsts -c $bench $options"

build/RISCV/gem5.opt --debug-flag=$debugflags --debug-file=$logfile ./configs/example/se.py --maxinsts=$maxinsts --startinsts=$startinsts --endinsts=$endinsts --stackbase=$stacktop --mmapend=$mmapend --ckptinsts=$ckptinsts -c $bench "$options"
