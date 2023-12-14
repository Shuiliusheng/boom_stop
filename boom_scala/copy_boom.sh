#!/bin/bash

if [[ $# < 1 ]]; then
	echo "parameters is not enough"
	echo "./copy_to_boom boom_dir"
	echo "example: ./copy_to_boom.sh ~/vivado-risc-v/generators/riscv-boom/"
	exit
fi

boom_dir=$1"src/main/scala"

if [[ ! -d $boom_dir ]]; then
	echo $boom_dir" is not exist!"
	exit
fi

if [[ ! -d $boom_dir"/common" ]]; then
	echo $boom_dir"/"$common " is not exist!"
	exit
fi

if [[ ! -d "common" ]]; then
	echo "./common is not exist!"
	exit
fi

echo rm $boom_dir/* -r
rm $boom_dir/* -r

echo cp ./common $boom_dir/common -r
cp ./common $boom_dir/common -r

echo cp ./exu $boom_dir/exu -r
cp ./exu $boom_dir/exu -r

echo cp ./ifu $boom_dir/ifu -r
cp ./ifu $boom_dir/ifu -r

echo cp ./lsu $boom_dir/lsu -r
cp ./lsu $boom_dir/lsu -r

echo cp ./util $boom_dir/util -r
cp ./util $boom_dir/util -r