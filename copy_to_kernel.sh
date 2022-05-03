#!/bin/bash

if [[ $# < 1 ]]; then
	echo "parameters is not enough"
	echo "./copy_to_kernel linux_kernel_dir"
	echo "example: ./copy_to_kernel.sh ~/vivado-risc-v/linux-stable/"
	exit
fi
src_dir="linux_arch_riscv" 
linux_dir=$1"arch/riscv"

if [[ ! -e $linux_dir ]]; then
	echo $linux_dir " is not exist!"
	exit
fi

if [[ ! -e $src_dir ]]; then
	echo $src_dir " is not exist!"
	exit
fi

cd $src_dir
for file in `find -name "*"`;
do
	if [[ -f $file ]]; then
		if [[ -e $linux_dir/$file ]]; then
			echo cp $file $linux_dir/$file
			rm $linux_dir/$file
			cp $file $linux_dir/$file
		fi
	fi
done
