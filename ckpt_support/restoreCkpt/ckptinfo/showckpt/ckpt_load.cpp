#include "ckptinfo.h"

typedef struct{
    uint64_t addr;
    // uint64_t size;
    uint64_t data;
}LoadInfo;
//默认均为8B

typedef struct{
    uint64_t start;
    uint64_t simNum;
    uint64_t exitpc;
    uint64_t exit_cause;
}SimInfo;

uint64_t read_ckptsyscall(FILE *fp)
{
    uint64_t filesize, allocsize, alloc_vaddr, nowplace;
    nowplace = ftell(fp);
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp) - nowplace;
    fseek(fp, nowplace, SEEK_SET);

    allocsize = filesize + (4096-filesize%4096);
    alloc_vaddr = (uint64_t)mmap((void *)0x2000000, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);    
    
    fread((void *)alloc_vaddr, filesize, 1, fp);
    uint64_t totalcallnum = *((uint64_t *)alloc_vaddr);
    printf("--- syscall totalcallnum: %ld ---\n", totalcallnum);
    return alloc_vaddr;
}


void read_ckptinfo(char ckptinfo[])
{
    uint64_t npc=0, temp=0, numinfos=0;
    SimInfo siminfo;
    FILE *p=NULL;
    p = fopen(ckptinfo,"rb");
    if(p == NULL){
        printf("cannot open %s to read\n", ckptinfo);
        exit(1);
    }

    fread(&numinfos, 8, 1, p);
    temp = 0;
    MemRangeInfo textinfo;
    for(int i=0;i<numinfos;i++){
        fread(&textinfo, sizeof(MemRangeInfo), 1, p);
        temp += textinfo.size;
        printf("text range %d: (0x%lx 0x%lx), %ld KB\n", i, textinfo.addr, textinfo.addr+textinfo.size, textinfo.size/1024);
    }
    printf("text range total size: %ld KB\n\n\n", temp/1024);


    fread(&siminfo, sizeof(siminfo), 1, p);
    uint64_t warmup = siminfo.simNum >> 32;
    siminfo.simNum = (siminfo.simNum << 32) >> 32;

    //step 1: read npc
    fread(&npc, 8, 1, p);
    printf("sim slice info, start: %ld, npc: 0x%lx, simNum: %ld, rawLength: %ld, warmup: %ld, exitpc: 0x%lx, cause: %ld\n\n", siminfo.start, npc, siminfo.simNum, siminfo.exit_cause>>2, warmup, siminfo.exitpc, siminfo.exit_cause%4);

    uint64_t intregs[32], fpregs[32];
    fread(&intregs[0], 8, 32, p);
    fread(&fpregs[0], 8, 32, p);

    //step 3: read memory range information and map these ranges
    MemRangeInfo memrange;
    uint64_t mrange_num=0;
    temp = 0;
    fread(&mrange_num, 8, 1, p);
    for(int i=0;i<mrange_num;i++){
        fread(&memrange, sizeof(MemRangeInfo), 1, p);
        temp += memrange.size;
        printf("mem range %d: (0x%lx 0x%lx), %ld KB\n", i, memrange.addr, memrange.addr+memrange.size, memrange.size/1024);
    }
    printf("mem range total size: %ld KB\n\n\n", temp/1024);

    printf("--- integer registers ---\n");
    for(int i=0;i<32;i+=4) {
        printf("x%2d: 0x%20lx, ", i, intregs[i]);
        printf("x%2d: 0x%20lx, ", i+1, intregs[i+1]);
        printf("x%2d: 0x%20lx, ", i+2, intregs[i+2]);
        printf("x%2d: 0x%20lx\n", i+3, intregs[i+3]);
    }
    printf("--- float registers ---\n");
    for(int i=0;i<32;i+=4) {
        printf("f%2d: 0x%20lx, ", i, fpregs[i]);
        printf("f%2d: 0x%20lx, ", i+1, fpregs[i+1]);
        printf("f%2d: 0x%20lx, ", i+2, fpregs[i+2]);
        printf("f%2d: 0x%20lx\n", i+3, fpregs[i+3]);
    }
    printf("\n\n");

    //step 4: read first load information, and store these data to memory
    uint64_t loadnum = 0;
    fread(&loadnum, 8, 1, p);
    printf("--- first load num: %ld ---\n\n", loadnum);
    LoadInfo *linfos = (LoadInfo *)malloc(sizeof(LoadInfo)*loadnum);
    fread(&linfos[0], sizeof(LoadInfo), loadnum, p);
    // for(int i=0;i<loadnum;i++){
    //     printf("load %d: 0x%lx 0x%lx\n", i, linfos[i].addr,  linfos[i].data);
    // }
    free(linfos);
    
    //step5: 加载syscall的执行信息到内存中
    uint64_t alloc_vaddr = read_ckptsyscall(p);
    fclose(p);
}
