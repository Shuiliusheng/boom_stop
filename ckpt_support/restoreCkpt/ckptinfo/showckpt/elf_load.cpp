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

uint64_t loadelf(char * progname)
{
	Elf64_Ehdr ehdr;
	Elf64_Phdr phdr;
	FILE *fp = fopen(progname, "rb");
	if (fp == 0) {
		printf("cannot open %s\n", progname);
        exit(1);
	}
	for(int i=0; i<ehdr.e_phnum; i++) 
    {
		fseek(fp, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
		fread(&phdr, sizeof(phdr), 1, fp);
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
            break;
        }
	}
    fclose(fp);
    return 0;
}