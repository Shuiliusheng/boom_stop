#!/bin/bash
riscv_toolchain="riscv64-unknown-linux-gnu"

${riscv_toolchain}-gcc -I./env -I./common  -DPREALLOCATE=1 -mcmodel=medany -std=gnu99 -O2 -ffast-math -fno-common -fno-builtin-printf -fno-tree-loop-distribute-patterns -o test.riscv ./test.c ./common/syscalls.c ./common/crt.S -static -nostdlib -nostartfiles -lm -lgcc -T ./common/test.ld

${riscv_toolchain}-objdump --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data test.riscv > test.dump
