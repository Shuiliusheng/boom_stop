#include "ckptinfo.h"
#include <vector>
using namespace std;

void initMidJmpPlace()
{
    uint64_t vaddr = (uint64_t)mmap((void*)TPoint1, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
    if(vaddr != TPoint1){
        printf("cannot alloc memory in 0x%lx for TPoint1, alloc: 0x%lx\n", TPoint1, vaddr);
        exit(0);
    }

    vaddr = (uint64_t)mmap((void*)TPoint2, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
    if(vaddr != TPoint2){
        printf("cannot alloc memory in 0x%lx for TPoint1, alloc: 0x%lx\n", TPoint2, vaddr);
        exit(0);
    }
}

uint32_t createJAL(uint64_t base, uint64_t target) {
    uint32_t offset = target - base;
    uint32_t op = 0x6f, rd = 0, inst = 0;
    uint32_t imm20=0, v12 = 0, v11 = 0, v1 = 0, v20 = 0;
    v1 = (offset >> 1) % 1024;
    v11 = (offset >> 11) % 2;
    v12 = (offset >> 12) % 256;
    v20 = (offset >> 20) % 2;   
    imm20 = v12 + (v11 << 8) + (v1 << 9) + (v20 << 19);
    inst = op + (rd << 7) + (imm20 << 12); 
    return inst;
}

bool setJmp(uint64_t instaddr, uint64_t target) {
    uint32_t temp = createJAL(instaddr, target);
    if(temp == 0){
        return false;
    }
    *(uint32_t *)instaddr = temp;
    return true;
}

vector<MemRangeInfo> freeRanges;
void getRangeInfo(char filename[]) {
    FILE *p = fopen(filename,"rb");
    vector<MemRangeInfo> ranges;
    uint64_t num = 0;
    MemRangeInfo range;
    fread(&num, 8, 1, p);
    for(int i=0;i<num;i++){
        fread(&range, sizeof(MemRangeInfo), 1, p);
        ranges.push_back(range);
    }

    uint64_t offset = (8+num*16) + 5*8 + 64*8;
    fseek(p, offset, SEEK_SET);
    fread(&num, 8, 1, p);
    for(int i=0;i<num;i++){
        fread(&range, sizeof(MemRangeInfo), 1, p);
        ranges.push_back(range);
    }
    fclose(p);

    for(int i=1;i<ranges.size();i++){
        range.addr = ranges[i-1].addr + ranges[i-1].size;
        range.size = ranges[i].addr - range.addr;
        if(ranges[i].addr > text_seg.addr + text_seg.size) break;
        if(range.size > 0) {
            freeRanges.push_back(range);
        }
    }
    ranges.clear();
}

vector<uint64_t> midpoints;
void findMidPoints(uint64_t base, uint64_t maxtarget)
{
    if(maxtarget < base + MaxJALOffset) {
        return ;
    }
    uint64_t maxaddr = base + MaxJALOffset, jmpmax = 0, temp=0, temp1=0;
    int i = 0;
    while (1) {
        for(;i<freeRanges.size() && (maxaddr>=freeRanges[i].addr);i++);
        temp = freeRanges[i-1].addr + freeRanges[i-1].size - 0x100;
        temp1 = temp + MaxJALOffset;
        if(i==0 || (temp1 < freeRanges[i].addr && temp1 < maxtarget)){
            printf("cannot found jmp path from 0x%x to 0x%x\n", temp1, maxtarget);
            exit(1);
        }
        while(1) {
            jmpmax = (temp > maxaddr) ? maxaddr : temp;
            maxaddr = jmpmax + MaxJALOffset;
            midpoints.push_back(jmpmax);
            if(temp <= maxaddr) break;
            if(maxaddr > maxtarget) break;
        }
        if(maxaddr > maxtarget) break;
    }
    freeRanges.clear();
}


void replaceSyscall(vector<uint64_t> &pcs) 
{
    // replace syscall with jmp to TPoint2 + 4
    RunningInfo *runinfo = (RunningInfo *)RunningInfoAddr;
    setJmp(TPoint2+4, TPoint1+4);
    setJmp(TPoint1+4, takeOverAddr);
    for(int i=1;i<midpoints.size();i++){
        setJmp(midpoints[i]+4, midpoints[i-1]+4);
    }

    for(int i=1; i<pcs.size(); i++) {
        for(int c=0;c<midpoints.size();c++){
            if(pcs[i] - (midpoints[c]+4) < MaxJALOffset){
                if(i == pcs.size()-1)
                    runinfo->exitJmpInst = createJAL(pcs[i], midpoints[c]+4);
                else
                    setJmp(pcs[i], midpoints[c]+4);
            }
        }
    }
    asm volatile("fence.i  ");
}

void produceSysRet(vector<uint64_t> &pcs)
{
    //set jmp to syscall ret place
    RunningInfo *runinfo = (RunningInfo *)RunningInfoAddr;
    JmpRepInfo *infos = (JmpRepInfo *)malloc(sizeof(JmpRepInfo) * pcs.size());
    for(int i=0; i<pcs.size(); i++) {
        int max = 0;
        for(max = 0; max < midpoints.size(); max++){
            if(pcs[i]+4 - midpoints[max] < MaxJALOffset) {
                break;
            }
        }
        infos[i].infos = (JmpInfo *)malloc(sizeof(JmpInfo)*(max+1));
        for(int c=0; c<max; c++) {
            infos[i].infos[c].addr = midpoints[c];
            infos[i].infos[c].inst = createJAL(midpoints[c], midpoints[c+1]);
        }
        infos[i].infos[max].addr = midpoints[max];
        infos[i].infos[max].inst = createJAL(midpoints[max], pcs[i]+4);
        infos[i].num = max + 1;
    }
    runinfo->sysJmpinfos = infos;
}

void produceJmpInst(uint64_t npc)
{
    vector<uint64_t> pcs;
    pcs.push_back(npc-4);

    RunningInfo *runinfo = (RunningInfo *)RunningInfoAddr;
    SyscallInfo *sinfos = NULL;
    for(int i=0; i<runinfo->totalcallnum; i++) {
        sinfos = (SyscallInfo *)(runinfo->syscall_info_addr + 8 + i*sizeof(SyscallInfo));
        pcs.push_back(sinfos->pc);
    }
    pcs.push_back(runinfo->exitpc);

    uint64_t maxtarget = 0;
    for(int i=0;i<pcs.size();i++){
        if(maxtarget < pcs[i]) maxtarget = pcs[i];
    }
    midpoints.push_back(TPoint2);

    findMidPoints(TPoint2, maxtarget);
    replaceSyscall(pcs);
    produceSysRet(pcs);
    setJmp(TPoint1, TPoint2);

    pcs.clear();
    midpoints.clear();
}

void updateJmpInst(JmpRepInfo &info)
{
    for(int i=0;i<info.num;i++){
        *(uint32_t *)(info.infos[i].addr) = info.infos[i].inst;
    }
    asm volatile("fence.i");
}