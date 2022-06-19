#include "ckptinfo.h"


int main(int argc, char **argv)
{
    if(argc < 2){
        printf("parameters are not enough!\n");
        printf("./readckpt.riscv ckptinfo.log exe\n");
        return 0;
    }

    loadelf(argv[2]);
    read_ckptinfo(argv[1]);
    return 0;
}