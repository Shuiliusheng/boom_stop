#include "info.h"

typedef struct{
    uint64_t addr;
    uint64_t size;
    uint64_t data;
}LoadInfo;


void read_ckptsyscall(char filename[])
{
    uint64_t filesize, allocsize, alloc_vaddr;
    FILE *fp=NULL;
    fp = fopen(filename,"rb");
    if(fp == NULL) {
        printf("cannot open %s to read\n", filename);
        exit(1);
    }
    
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    allocsize = filesize + (4096-filesize%4096);
    alloc_vaddr = (uint64_t)mmap((void *)0x2000000, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_FIXED, -1, 0);    
    
    if (fread((void *)alloc_vaddr, filesize, 1, fp) != 1) {
        printf("cannot read file: %s\n", filename);
        exit(1);
    }
    fclose(fp);

    RunningInfo *runinfo = (RunningInfo *)RunningInfoAddr;
    runinfo->syscall_info_addr = alloc_vaddr;
    runinfo->nowcallnum = 0;
    runinfo->totalcallnum = *((uint64_t *)alloc_vaddr);

    printf("--- step 5, read syscall information and map them to memory, totalcallnum: %d ---\n", runinfo->totalcallnum);
    if(ShowLog)
        printf("syscallnum: %ld, alloc_mem: (0x%lx 0x%lx)\n", runinfo->totalcallnum, alloc_vaddr,  alloc_vaddr + allocsize);
}



void read_ckptinfo(char ckptinfo[], char ckpt_sysinfo[])
{
    uint64_t npc, mrange_num=0, loadnum = 0;
    MemRangeInfo memrange, extra;
    LoadInfo loadinfo;
    FILE *p=NULL;
    p = fopen(ckptinfo,"rb");
    if(p == NULL){
        printf("cannot open %s to read\n", ckptinfo);
        exit(1);
    }

    //step 1: read npc
    fread(&npc, 8, 1, p);
    printf("--- step 1, read npc: 0x%lx ---\n", npc);


    //step 2: read integer and float registers
    RunningInfo *runinfo = (RunningInfo *)RunningInfoAddr;
    fread(&runinfo->intregs[0], 8, 32, p);
    fread(&runinfo->fpregs[0], 8, 32, p);
    printf("--- step 2, read integer and float registers ---\n");

    //step 3: read memory range information and map these ranges
    fread(&mrange_num, 8, 1, p);
    printf("--- step 3, read memory range information and do map, range num: %d ---\n", mrange_num);

    for(int i=0;i<mrange_num;i++){
        fread(&memrange, sizeof(memrange), 1, p);

        extra.size = 0;
        extra.addr = 0;
        uint64_t msaddr = memrange.addr, meaddr = memrange.addr + memrange.size;
        uint64_t tsaddr = text_seg.addr, teaddr = text_seg.addr + text_seg.size;
        //delete the range that covered by text segment
        if(msaddr < tsaddr && meaddr > tsaddr) {
            memrange.size = tsaddr - msaddr;
            if(meaddr > teaddr) {
                extra.addr = teaddr;
                extra.size = meaddr - teaddr;
            }
        }
        else if(msaddr >= tsaddr && msaddr <= teaddr) {
            if(meaddr <= teaddr) {
                memrange.size = 0;
            }
            else{
                memrange.addr = teaddr;
                memrange.size = meaddr - teaddr;
            }
        }

        if(memrange.size !=0){
            if(memrange.addr < 0xfffffffff){ // a small limit
                int* arr = static_cast<int*>(mmap((void *)memrange.addr, memrange.size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_FIXED, 0, 0));
                if(ShowLog)
                    printf("map range: (0x%lx, 0x%lx), mapped addr: 0x%lx\n", memrange.addr, memrange.addr + memrange.size, arr);
                assert(memrange.addr == (uint64_t)arr);  
            }
        }

        if(extra.size !=0){
            int* arr1 = static_cast<int*>(mmap((void *)extra.addr, extra.size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_FIXED, 0, 0));
            if(ShowLog)
                printf("map range: (0x%lx, 0x%lx), mapped addr: 0x%lx\n", extra.addr, extra.addr + extra.size, arr1);
            assert(extra.addr == (uint64_t)arr1); 
        }
    }

    //step 4: read first load information, and store these data to memory
    fread(&loadnum, 8, 1, p);
    printf("--- step 4, read first load information and init these loads, load num: %d ---\n", loadnum);
    for(int i=0;i<loadnum;i++){
        fread(&loadinfo, sizeof(loadinfo), 1, p);
        if(loadinfo.addr < 0xfffffffff){ //防止写入到栈中，影响程序执行  
            switch(loadinfo.size) {
                case 1: *((uint8_t *)loadinfo.addr) = (uint8_t)loadinfo.data; break;
                case 2: *((uint16_t *)loadinfo.addr) = (uint16_t)loadinfo.data; break;
                case 4: *((uint32_t *)loadinfo.addr) = (uint32_t)loadinfo.data; break;
                case 8: *((uint64_t *)loadinfo.addr) = loadinfo.data; break;
            }
        }
    }
    fclose(p);
    
    //step5: 加载syscall的执行信息到内存中
    read_ckptsyscall(ckpt_sysinfo);

    //step6: init npc and takeover_syscall addr to temp register
    printf("--- step 6, init npc to rtemp(5) and takeover_syscall addr to rtemp(6) ---\n");
    WriteTemp("0", npc);
    uint64_t takeover_addr = ShowLog ? TakeOverAddrTrue : TakeOverAddrFalse;
    WriteTemp("1", takeover_addr);

    //step7: save registers data of boot program 
    printf("--- step 789, save registers data of boot program, set testing program registers, start testing ---\n");
    Save_int_regs(OldIntRegAddr);

    //step8: set the testing program's register information
    Load_regs(StoreIntRegAddr);

    //step9: start the testing program
    JmpTemp("0");
}
