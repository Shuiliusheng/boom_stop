#!/bin/bash


./build/RISCV/gem5.opt ./configs/example/se.py --cpu-type=DerivO3CPU --caches --mem-size=8GB --l1d_size=64kB --l1d_assoc=8 --l1i_size=16kB --l1i_assoc=4 --l2cache --l2_size=2MB --l2_assoc=16 --fast-forward=38000000 -c ckptinfo/readckpt/readckpt.riscv '--options=ckptinfo/info/gcctemp1/gcc_ckpt_1050000000.info ckptinfo/info/gcctemp1/gcc_ckpt_syscall_1050000000.info ckptinfo/test/gcc.riscv'
