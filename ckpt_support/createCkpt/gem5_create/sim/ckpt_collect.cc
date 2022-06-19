#include "sim/ckpt_collect.hh"

vector<CkptInfo *> pendingCkpts;
CkptSettings ckptsettings;
int ckptidx = 0;

bool cmp(CkptCtrl &ctrl1, CkptCtrl &ctrl2) { 
  return ctrl1.start < ctrl2.start; 
}

void init_ckpt_settings(const char filename[])
{
  if(strlen(filename) < 1) return ;

  FILE *p=NULL;
  p=fopen(filename,"r");
  if(p==NULL) {
    printf("%s cannot be opened\n", filename);
    return ;
  }

  char str[100];
  uint64_t v1, v2, v3, v4;
  while(!feof(p)) {
    fgets(str, 100, p);
    if(strlen(str) < 1) break;
    str[strlen(str)-1] = '\0';

    string temp(str);
    uint64_t idx = temp.find(":");
    if(idx == temp.npos) {
      printf("%s", str);
      strcpy(str, "");
      continue;
    }
    if(temp.find("mmapend") != temp.npos) {
      sscanf(&str[idx+1], "%lld", &v1);
      printf("mmapend: 0x%lx\n", v1);
      ckptsettings.mmapend = v1;
    }
    else if(temp.find("stacktop") != temp.npos) {
      sscanf(&str[idx+1], "%lld", &v1);
      printf("stacktop: 0x%lx\n", v1);
      ckptsettings.stack_base = v1;
    }
    else if(temp.find("ckptprefix") != temp.npos) {
      idx = idx + 1;
      for(; idx<strlen(str) && str[idx] ==' '; idx++);
      printf("benchname: %s\n", &str[idx]);
      strcpy(ckptsettings.benchname,  &str[idx]);
    }
    else if(temp.find("ckptctrl") != temp.npos) {
      sscanf(&str[idx+1], "%lld %lld %lld %lld", &v1, &v2, &v3, &v4);
      CkptCtrl ctrl;
      for(int i=0; i<v4; i++) {
        ctrl.start = v1 + v2*i - v3;
        ctrl.end = ctrl.start + v2 + v3;
        ctrl.warmup = v3;
        ckptsettings.ctrls.push_back(ctrl);
      }
    }
    else{
      printf("%s", str);
    }
    strcpy(str, "");
  }
  sort(ckptsettings.ctrls.begin(), ckptsettings.ctrls.end(), cmp);
  for(int i=0;i<ckptsettings.ctrls.size();i++){
    printf("ckpt info, start: %lld, end: %lld, warmup: %lld\n", ckptsettings.ctrls[i].start, ckptsettings.ctrls[i].end, ckptsettings.ctrls[i].warmup);
  }
  fclose(p);
}


void addCkpt(uint64_t startnum, uint64_t length, uint64_t intregs[], uint64_t fpregs[], uint64_t pc, uint64_t npc, uint64_t instinfo[]) 
{
  uint64_t warmup = ckptsettings.ctrls[ckptidx-1].warmup;
  pendingCkpts.push_back(new CkptInfo(startnum, length, warmup, intregs, fpregs, pc, npc, ckptsettings.benchname, instinfo));
}

void ckpt_addload(uint64_t addr, uint8_t *data, unsigned char size) 
{
  for(int i=0;i<pendingCkpts.size();i++){
    pendingCkpts[i]->addload(addr, data, size);
  }
} 

void ckpt_addstore(uint64_t addr, unsigned char size) 
{
  for(int i=0;i<pendingCkpts.size();i++){
    pendingCkpts[i]->addstore(addr, size);
  }
} 

void ckpt_addinst(uint64_t addr) 
{
  for(int i=0;i<pendingCkpts.size();i++){
    pendingCkpts[i]->addinst(addr);
  }
} 

void ckpt_add_sysenter(uint64_t pc, uint32_t sysnum, vector<uint64_t> &params) 
{
  if(pendingCkpts.size() > 0) {
    pendingCkpts[pendingCkpts.size()-1]->add_sysenter(pc, sysnum, params);
  }
}

void ckpt_add_sysexe(uint64_t pc, uint64_t ret, uint64_t bufaddr, uint32_t bufsize, uint8_t data[]) 
{
  if(pendingCkpts.size() > 0) {
    pendingCkpts[pendingCkpts.size()-1]->add_sysexe(pc, ret, bufaddr, bufsize, data);
  }
}

void ckpt_add_sysret(uint64_t pc, string name, bool hasret, uint64_t ret)
{
  if(pendingCkpts.size() > 0) {
    pendingCkpts[pendingCkpts.size()-1]->add_sysret(pc, name, hasret, ret);
  }
}

void ckpt_detectOver(uint64_t exit_place, uint64_t exit_pc, uint64_t instinfo[]) 
{
  for (vector<CkptInfo *>::iterator it = pendingCkpts.begin(); it != pendingCkpts.end();) {
    if ((*it)->detectOver(exit_place, exit_pc, instinfo)) {
      printf("exit ckpt start with: %d\n", (*it)->startnum);
      delete *it;
      it = pendingCkpts.erase(it);
    } else {
      ++it;
    }
  }
}

bool hasValidCkpt() 
{
    if(pendingCkpts.size()!= 0) {
        return true;
    }
    else{
        return false;
    }
}

bool isCkptStart(uint64_t simNum, uint64_t &length)
{
    if(ckptidx >= ckptsettings.ctrls.size()) {
        return false;
    }

    if(simNum == ckptsettings.ctrls[ckptidx].start) {
        length = ckptsettings.ctrls[ckptidx].end - ckptsettings.ctrls[ckptidx].start;
        ckptidx++;
        return true;
    }
    else{
        return false;
    }
}





