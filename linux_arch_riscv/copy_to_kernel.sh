#!/bin/bash

if [[ $# < 1 ]]; then
	echo "parameters is not enough"
	echo "./copy_to_kernel linux_kernel_dir"
	echo "example: ./copy_to_kernel.sh ~/vivado-risc-v/linux-stable/"
	exit
fi

linux_dir=$1"arch/riscv"

processfile=include/asm/processor.h
offsetfile=kernel/asm-offsets.c
entryfile=kernel/entry.S

if [[ ! -e $linux_dir"/"$processfile ]]; then
	echo $linux_dir"/"$processfile " is not exist!"
	exit
fi

if [[ ! -e $linux_dir"/"$offsetfile ]]; then
	echo $linux_dir"/"$offsetfile " is not exist!"
	exit
fi

if [[ ! -e $linux_dir"/"$entryfile ]]; then
	echo $linux_dir"/"$entryfile " is not exist!"
	exit
fi


if [[ ! -e $processfile ]]; then
	echo $processfile " is not exist!"
	exit
fi

if [[ ! -e $offsetfile ]]; then
	echo $offsetfile " is not exist!"
	exit
fi

if [[ ! -e $entryfile ]]; then
	echo $entryfile " is not exist!"
	exit
fi

echo cp $processfile $linux_dir/$processfile
cp $processfile $linux_dir/$processfile

echo cp $offsetfile $linux_dir/$offsetfile
cp $offsetfile $linux_dir/$offsetfile

echo cp $entryfile $linux_dir/$entryfile
cp $entryfile $linux_dir/$entryfile
