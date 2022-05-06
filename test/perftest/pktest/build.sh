riscv64-unknown-linux-gnu-gcc pftest.c -static -o pftest.riscv
riscv64-linux-gnu-objdump -d pftest.riscv >test.s
grep -rn exit_fuc test.s -A 6
