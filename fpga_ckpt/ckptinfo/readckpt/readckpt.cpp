#include "info.h"


int main(int argc, char **argv)
{
    if(argc < 3){
        printf("parameters are not enough!\n");
        printf("./read ckptinfo.log ckpt_syscallinfo.log \n");
        return 0;
    }

    uint64_t alloc_vaddr = (uint64_t)mmap((void*)RunningInfoAddr, 4096*2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
    if(alloc_vaddr != RunningInfoAddr){
        printf("cannot alloc memory in 0x%lx for record runtime information, alloc: 0x%lx\n", RunningInfoAddr, alloc_vaddr);
        return 0;
    }
    loadelf(argv[3], argv[1]);
    read_ckptinfo(argv[1], argv[2]);
    return 0;
}