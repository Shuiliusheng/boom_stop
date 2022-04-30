#!/bin/bash

if [[ $# < 1 ]]; then
    echo "parameters are not enough!"
    exit
fi

echo "build $1"
name=`ls $1 | awk -F '.' '{print $1}'`
riscv64-unknown-linux-gnu-gcc $1 -static -O2 -o $name.riscv