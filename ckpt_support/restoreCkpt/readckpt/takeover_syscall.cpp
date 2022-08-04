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
    
    if (runinfo->nowcallnum >= runinfo->totalcallnum){
        printf("syscall is overflow, exit.\n");
        printf("test program user running info, cycles: %ld, insts: %ld\n", runinfo->cycles, runinfo->insts);
        printf("test program total running info, cycles: %ld, insts: %ld\n", cycles - runinfo->startcycles, insts - runinfo->startinsts);
        exit(0);
    }

    uint64_t infoaddr = runinfo->syscall_info_addr + 8 + runinfo->totalcallnum*4;
    uint32_t *sysidxs = (uint32_t *)(runinfo->syscall_info_addr + 8);
    SyscallInfo *infos = (SyscallInfo *)(infoaddr + sysidxs[runinfo->nowcallnum]*sizeof(SyscallInfo));
    runinfo->intregs[10] = infos->p0;

    uint64_t sysnum = infos->num >> 32;
    // printf("syscall %d: %d\n", runinfo->nowcallnum, sysnum);
    if(runinfo->intregs[17] != sysnum){
        printf("syscall num is wrong! callnum: 0x%lx, recorded num: 0x%lx, 0x%lx\n", runinfo->intregs[17], infos->pc, sysnum);
        exit(0);
    }

    //this syscall has data to process
    uint64_t data_offset = infos->data_offset >> 8;
    if (data_offset != 0xffffffff){
        uint64_t data_size = (infos->num<<32) >> 32;
        uint64_t bufaddr = *(uint64_t *)(runinfo->syscall_info_addr + data_offset);
        if(bufaddr != 0) {   //these syscall also need to write data to bufaddr
            unsigned char *dst = (unsigned char *)bufaddr;
            unsigned char *data = (unsigned char *)(runinfo->syscall_info_addr + data_offset + 8);
            for(int c=0;c<data_size;c++){
                dst[c] = data[c];
            }
        }
    }
    uint64_t hasret = infos->data_offset % 256;
    if(hasret) {
        runinfo->intregs[10] = infos->ret;
    }

    runinfo->nowcallnum ++;

    if(runinfo->nowcallnum == runinfo->totalcallnum && runinfo->exit_cause == Cause_ExitInst) {
        printf("--- exit the last syscall %d, and replace exit pc (0x%lx) with jmp ---\n", runinfo->nowcallnum, runinfo->exitpc);
        *((uint32_t *)runinfo->exitpc) = ECall_Replace;
	    asm volatile("fence.i  ");
    }

    uint64_t npc = infos->pc + 4;
    SetTempReg(npc, 2);

    runinfo->lastcycles = __csrr_cycle();
    runinfo->lastinsts = __csrr_instret();

    Context_Operation("ld x", StoreIntRegAddr);
    JmpTempReg(2);
}
