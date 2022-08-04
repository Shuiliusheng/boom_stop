riscv64-unknown-linux-gnu-g++ perf.cpp trace.cpp util.cpp -static -o perf.riscv
riscv64-unknown-linux-gnu-g++ test.cpp -static -o test.riscv
cp perf.riscv ~
cp test.riscv ~
