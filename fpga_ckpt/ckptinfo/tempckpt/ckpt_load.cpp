#include "info.h"

typedef struct{
    uint64_t addr;
    uint64_t size;
    uint64_t data;
}LoadInfo;

typedef struct{
    uint64_t start;
    uint64_t simNum;
    uint64_t exitpc;
    uint64_t exit_cause;
}SimInfo;

void read_ckptsyscall(char filename[])
{
    uint64_t  totalcallnum;
    FILE *fp=NULL;
    fp = fopen(filename,"rb");
    if(fp == NULL) {
        printf("cannot open %s to read\n", filename);
        exit(1);
    }
    
    fread(&totalcallnum, sizeof(uint64_t), 1, fp);
    fclose(fp);
    printf("--- step 5, syscall totalcallnum: %d ---\n", totalcallnum);
}



void read_ckptinfo(char ckptinfo[], char ckpt_sysinfo[])
{
    uint64_t npc, mrange_num=0, loadnum = 0, temp=0, intregs[32], fpregs[32];
    MemRangeInfo memrange, extra;
    LoadInfo loadinfo;
    SimInfo siminfo;

    FILE *p=NULL;
    p = fopen(ckptinfo,"rb");
    if(p == NULL){
        printf("cannot open %s to read\n", ckptinfo);
        exit(1);
    }

    fread(&temp, 8, 1, p);
    fseek(p, 16*temp+8, SEEK_SET);

    fread(&siminfo, sizeof(siminfo), 1, p);
    printf("siminfo, start: %d, simNum: %d, exitpc: 0x%lx, cause: %d\n", siminfo.start, siminfo.simNum, siminfo.exitpc, siminfo.exit_cause);
   
    //step 1: read npc
    fread(&npc, 8, 1, p);
    printf("--- step 1, read npc: 0x%lx ---\n", npc);


    //step 2: read integer and float registers
    fread(&intregs[0], 8, 32, p);
    fread(&fpregs[0], 8, 32, p);
    printf("--- step 2, read integer and float registers ---\n");

    //step 3: read memory range information and map these ranges
    fread(&mrange_num, 8, 1, p);
    printf("--- step 3, read memory range information and do map, range num: %d ---\n", mrange_num);
    MemRangeInfo *minfos = (MemRangeInfo *)malloc(sizeof(MemRangeInfo)*mrange_num);
    fread(&minfos[0], sizeof(MemRangeInfo), mrange_num, p);
    for(int i=0;i<mrange_num;i++){
        //fread(&memrange, sizeof(memrange), 1, p);
        memrange = minfos[i];
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
            printf("map range %d: (0x%lx, 0x%lx)\n", i, memrange.addr, memrange.addr + memrange.size);
        }

        if(extra.size !=0){
            printf("map range %d: (0x%lx, 0x%lx)\n", i, extra.addr, extra.addr + extra.size);
        }
    }
    free(minfos);

    //step 4: read first load information, and store these data to memory
    fread(&loadnum, 8, 1, p);
    printf("--- step 4, read first load information and init these loads, load num: %d ---\n", loadnum);
    if(showload){
        LoadInfo *linfos = (LoadInfo *)malloc(sizeof(LoadInfo)*loadnum);
        fread(&linfos[0], sizeof(LoadInfo), loadnum, p);
        for(int j=0;j<loadnum;j++){
            printf("load %d: addr: 0x%lx, size: %d, data: 0x%lx\n", j, linfos[j].addr, linfos[j].size, linfos[j].data);
        }
        free(linfos);
    }
    fclose(p);
    
    //step5: 加载syscall的执行信息到内存中
    read_ckptsyscall(ckpt_sysinfo);
}
