#include "ckptinfo.h"

typedef struct {
  uint8_t  e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
} Elf64_Phdr;

typedef struct {
  uint32_t sh_name;
  uint32_t sh_type;
  uint64_t sh_flags;
  uint64_t sh_addr;
  uint64_t sh_offset;
  uint64_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  uint64_t sh_addralign;
  uint64_t sh_entsize;
} Elf64_Shdr;


#define PF_X 1
#define PF_W 2
#define PF_R 4

MemRangeInfo text_seg;

static inline int get_prot(uint32_t p_flags)
{
  int prot_x = (p_flags & PF_X) ? PROT_EXEC  : PROT_NONE;
  int prot_w = (p_flags & PF_W) ? PROT_WRITE : PROT_NONE;
  int prot_r = (p_flags & PF_R) ? PROT_READ  : PROT_NONE;
  return (prot_x | prot_w | prot_r);
}

uint64_t loadelf(char * progname, char *ckptinfo)
{
	Elf64_Ehdr ehdr;
	Elf64_Phdr phdr;
    uint64_t startvaddr;
    uint64_t cycles[2], insts[2];
    cycles[0] = __csrr_cycle();
    insts[0] = __csrr_instret();

    FILE *fp1 = fopen(ckptinfo, "rb");
    if (fp1 == 0) {
		printf("cannot open %s\n", ckptinfo);
        exit(1);
	}
	FILE *fp = fopen(progname, "rb");
	if (fp == 0) {
		printf("cannot open %s\n", progname);
        exit(1);
	}

    MemRangeInfo textinfo;
    uint64_t numinfos = 0;
    fread(&numinfos, 8, 1, fp1);
    fread(&ehdr, sizeof(ehdr), 1, fp);
    printf("textinfo: %d\n", numinfos);

	for(int i=0; i<ehdr.e_phnum; i++) 
    {
		fseek(fp, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
		fread(&phdr, sizeof(phdr), 1, fp);
        //PT_LOAD 1
        if(phdr.p_type == 1 && phdr.p_memsz && phdr.p_filesz > 0){
            uint64_t prepad = phdr.p_vaddr % 4096; 
            uint64_t vaddr = phdr.p_vaddr - prepad;
            uint64_t postpad = 4096 - (phdr.p_vaddr + phdr.p_memsz) % 4096;
            uint64_t pmemsz = (phdr.p_vaddr + phdr.p_memsz + postpad) - vaddr;
            int prot = get_prot(phdr.p_flags);
            if(!(prot & PROT_EXEC)) {
                continue;
            }	
            
            text_seg.addr = vaddr;
            text_seg.size = pmemsz;	

            uint64_t alloc_vaddr = (uint64_t)mmap((void*)vaddr, pmemsz, prot | PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANON, -1, 0);
            printf("--- load elf: alloc text memory: (0x%lx, 0x%lx) ---\n", alloc_vaddr, alloc_vaddr + pmemsz);	
            if(alloc_vaddr != vaddr){
                printf("required vaddr: 0x%lx, alloc vaddr: 0x%lx\n", vaddr, alloc_vaddr);	
                exit(1);
            }
                
            

            //仅加载被使用到的代码段
            for(int i=0;i<numinfos;i++){
                printf("load text range: %d\r", i);
                fread(&textinfo, sizeof(MemRangeInfo), 1, fp1);
                unsigned int offset = textinfo.addr - phdr.p_vaddr + phdr.p_offset;
                fseek(fp, offset, SEEK_SET);
                fread((char *)textinfo.addr, textinfo.size, 1, fp); 
            }

            if(numinfos==0) {
                //find text segment and replace ecall with jmp rtemp
                Elf64_Shdr string_shdr, shdr;
                fseek(fp, ehdr.e_shoff + ehdr.e_shstrndx * ehdr.e_shentsize, SEEK_SET);
                fread(&string_shdr, sizeof(string_shdr), 1, fp);
                fseek(fp, string_shdr.sh_offset + string_shdr.sh_size, SEEK_SET);
                for (int c = 0; c < ehdr.e_shnum; c++) {
                    fseek(fp, ehdr.e_shoff + c * ehdr.e_shentsize, SEEK_SET);
                    fread(&shdr, sizeof(shdr), 1, fp);
                    
                    if(shdr.sh_size!=0 && shdr.sh_flags & 0x4){//SHF_EXECINSTR = 0x4
                        fseek(fp, shdr.sh_offset, SEEK_SET);
                        fread((void *)shdr.sh_addr, shdr.sh_size, 1, fp);
                    }
                }
            }
            break;
        }
	}
    fclose(fp);
    fclose(fp1);
    cycles[1] = __csrr_cycle();
    insts[1] = __csrr_instret();

    printf("load elf running info, cycles: %ld, insts: %ld\n", cycles[1]-cycles[0], insts[1]-insts[0]);
    return startvaddr;
}