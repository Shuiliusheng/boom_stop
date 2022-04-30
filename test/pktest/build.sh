riscv64-unknown-linux-gnu-gcc test.cpp -static -o test.riscv
riscv64-linux-gnu-objdump -d test.riscv >test.s