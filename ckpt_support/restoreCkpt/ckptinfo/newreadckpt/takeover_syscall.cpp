#include "ckptinfo.h"

extern unsigned long long __takeOverSys_Addr;
uint64_t takeOverAddr = (uint64_t)&__takeOverSys_Addr;

void takeoverSyscall()
{
    asm volatile("__takeOverSys_Addr: ");
    Context_Operation("sd x", StoreIntRegAddr);
    Load_necessary(OldIntRegAddr);
    
    RunningInfo *runinfo = (RunningInfo *)RunningInfoAddr;
    uint64_t cycles = 0, insts = 0;
    cycles = __csrr_cycle();
    insts = __csrr_instret();
    runinfo->cycles += (cycles - runinfo->lastcycles);
    runinfo->insts += (insts - runinfo->lastinsts);

    uint64_t saddr = runinfo->syscall_info_addr;
    SyscallInfo *infos = (SyscallInfo *)(saddr+8+runinfo->nowcallnum * sizeof(SyscallInfo));
    runinfo->intregs[10] = infos->p0;

    if (runinfo->nowcallnum >= runinfo->totalcallnum){
        printf("syscall is overflow, exit.\n");
        printf("test program user running info, cycles: %ld, insts: %ld\n", runinfo->cycles, runinfo->insts);
        printf("test program total running info, cycles: %ld, insts: %ld\n", cycles - runinfo->startcycles, insts - runinfo->startinsts);
        exit(0);
    }

    if(runinfo->intregs[17] != infos->num){
        printf("syscall num is wrong! callnum: 0x%lx, recorded num: 0x%lx, 0x%lx\n", runinfo->intregs[17], infos->pc, infos->num);
        exit(0);
    }

    //this syscall has data to process
    if (infos->data_offset != 0xffffffff){
        if (infos->num == 0x3f) {    //read
            unsigned char *data = (unsigned char *)(saddr + infos->data_offset);
            unsigned char *dst = (unsigned char *)runinfo->intregs[11];
            for(int c=0;c<infos->data_size;c++){
                dst[c] = data[c];
            }
        }
        else if(infos->num == 0x40) { //write
            char *dst = (char *)runinfo->intregs[11];
            if(runinfo->intregs[10] == 1){
                char t = dst[runinfo->intregs[12]-1];
                dst[runinfo->intregs[12]-1]='\0';
                printf("%s", (char *)dst);
                dst[runinfo->intregs[12]-1] = t;
                printf("\n");
            }
        }
        else if(infos->bufaddr != 0) {   //these syscall also need to write data to bufaddr
            unsigned char *dst = (unsigned char *)infos->bufaddr;
            unsigned char *data = (unsigned char *)(saddr + infos->data_offset);
            for(int c=0;c<infos->data_size;c++){
                dst[c] = data[c];
            }
        }
    }
    if(infos->hasret) {
        //modified the ret value
        runinfo->intregs[10] = infos->ret;
    }

    runinfo->nowcallnum ++;

    if(runinfo->nowcallnum == runinfo->totalcallnum) {
        printf("--- exit the last syscall %d, and replace exit pc (0x%lx) with jmp ---\n", runinfo->nowcallnum, runinfo->exitpc);
        *((uint32_t *)runinfo->exitpc) = runinfo->exitJmpInst;
	    asm volatile("fence.i");
    }

    updateJmpInst(runinfo->sysJmpinfos[runinfo->nowcallnum]);
    runinfo->lastcycles = __csrr_cycle();
    runinfo->lastinsts = __csrr_instret();

    Context_Operation("ld x", StoreIntRegAddr);
    StartJump();
}
