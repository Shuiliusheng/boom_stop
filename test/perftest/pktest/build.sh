riscv64-unknown-linux-gnu-gcc pftest.c -static -o pftest1.riscv
riscv64-linux-gnu-objdump -d pftest1.riscv >test.s
