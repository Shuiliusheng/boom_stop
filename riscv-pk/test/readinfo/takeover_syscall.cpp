#include "info.h"

typedef struct{
    uint64_t pc;
    uint64_t num;
    uint64_t p0, p1, p2;
    uint64_t hasret, ret;
    uint64_t bufaddr, data_offset, data_size;
}SyscallInfo;

void takeoverSyscall()
{
    Save_int_regs(StoreIntRegAddr);
    Load_necessary(OldIntRegAddr);

    RunningInfo *runinfo = (RunningInfo *)RunningInfoAddr;
    uint64_t saddr = runinfo->syscall_info_addr;
    if (runinfo->nowcallnum >= runinfo->totalcallnum){
        printf("syscall is overflow, exit!\n");
        exit(0);
    }

    SyscallInfo *infos = (SyscallInfo *)(saddr+8+runinfo->nowcallnum * sizeof(SyscallInfo));
    if(runinfo->intregs[17] != infos->num){
        printf("syscall num is wrong! callnum: 0x%lx, recorded num: 0x%lx, 0x%lx\n", runinfo->intregs[17], infos->pc, infos->num);
        exit(0);
    }

    if(ShowLog) 
        printf("syscall num: 0x%lx\n", infos->num);

    //this syscall has data to process
    if (infos->data_offset != 0xffffffff){
        if (infos->num == 0x3f) {    //read
            unsigned char *data = (unsigned char *)(saddr + infos->data_offset);
            unsigned char *dst = (unsigned char *)runinfo->intregs[11];
            for(int c=0;c<infos->data_size;c++){
                dst[c] = data[c];
            }

            if(ShowLog){
                printf("read, p0-p2: 0x%lx, 0x%lx, 0x%lx; record p0-p2: 0x%lx, 0x%lx, 0x%lx\n", runinfo->intregs[10], runinfo->intregs[11], runinfo->intregs[12], infos->p0, infos->p1, infos->p2);
            }
        }
        else if(infos->num == 0x40) { //write
            if(ShowLog){
                printf("write, p0-p2: 0x%lx, 0x%lx, 0x%lx; record p0-p2: 0x%lx, 0x%lx, 0x%lx\n", runinfo->intregs[10], runinfo->intregs[11], runinfo->intregs[12], infos->p0, infos->p1, infos->p2);
                if(runinfo->intregs[10] == 1){
                    printf("record data: ");
                    char *data = (char *)(saddr + infos->data_offset);
                    char t = data[runinfo->intregs[12]-1];
                    data[runinfo->intregs[12]-1]='\0';
                    printf("%s", (char *)data);
                    data[runinfo->intregs[12]-1] = t;
                }
            }
            char *dst = (char *)runinfo->intregs[11];
            if(runinfo->intregs[10] == 1){
                char t = dst[runinfo->intregs[12]-1];
                dst[runinfo->intregs[12]-1]='\0';
                printf("%s", (char *)dst);
                dst[runinfo->intregs[12]-1] = t;
            }
            printf("\n");
        }
        else if(infos->bufaddr != 0) {   //these syscall also need to write data to bufaddr
            unsigned char *dst = (unsigned char *)infos->bufaddr;
            unsigned char *data = (unsigned char *)(saddr + infos->data_offset);
            for(int c=0;c<infos->data_size;c++){
                dst[c] = data[c];
            }
            if(ShowLog){
                printf("p0-p2: 0x%lx, 0x%lx, 0x%lx; record p0-p2: 0x%lx, 0x%lx, 0x%lx, 0x%lx(bufaddr), %dB(datasize)\n", runinfo->intregs[10], runinfo->intregs[11], runinfo->intregs[12], infos->p0, infos->p1, infos->p2, infos->bufaddr, infos->data_size);
            }
        }
    }
    if(infos->hasret) {
        //modified the ret value
        runinfo->intregs[10] = infos->ret;
    }

    runinfo->nowcallnum ++;
    uint64_t npc = infos->pc + 4;
    WriteTemp("2", npc);
    Load_regs(StoreIntRegAddr);
    JmpTemp("2");
}
