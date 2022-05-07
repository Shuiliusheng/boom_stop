#include "info.h"

bool showload = false;

int main(int argc, char **argv)
{
    if(argc < 4){
        printf("parameters are not enough!\n");
        printf("./read ckptinfo.log ckpt_syscallinfo.log exe.riscv showload (0/1) \n");
        return 0;
    }
    if(argc == 5){
        showload = true;
    }
    
    uint64_t cycles[3], insts[3];
    cycles[0] = __csrr_cycle();
    insts[0] = __csrr_instret();
    loadelf(argv[3], argv[1]);
    cycles[1] = __csrr_cycle();
    insts[1] = __csrr_instret();
    read_ckptinfo(argv[1], argv[2]);
    cycles[2] = __csrr_cycle();
    insts[2] = __csrr_instret();


    printf("total runinfo, runcycles: %lu, insts: %lu\n", cycles[2]-cycles[0], insts[2]-insts[0]);
    printf(" -- loadelf runinfo, runcycles: %lu, insts: %lu\n", cycles[1]-cycles[0], insts[1]-insts[0]);
    printf(" -- read_ckptinfo runinfo, runcycles: %lu, insts: %lu\n", cycles[2]-cycles[1], insts[2]-insts[1]);
    return 0;
}