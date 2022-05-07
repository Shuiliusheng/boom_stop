#include "info.h"

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

MemRangeInfo data_seg, text_seg;



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

    FILE *fp1 = fopen(ckptinfo, "rb");
    if (fp1 == NULL) {
		printf("cannot open %s\n", progname);
        exit(1);
	}
    MemRangeInfo textinfo;
    uint64_t numinfos = 0;
    fread(&numinfos, 8, 1, fp1);
    printf("textinfo number: %d\n", numinfos);

	FILE *fp = fopen(progname, "r");
	if (fp == 0) {
		printf("cannot open %s\n", progname);
        exit(1);
	}

	if (fread(&ehdr, sizeof(ehdr), 1, fp) != 1) {
        printf("cannot read ehdr\n");
		exit(1);
	}
	
    startvaddr = ehdr.e_entry;
	for(int i=0; i<ehdr.e_phnum; i++) 
    {
		fseek(fp, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
		if (fread(&phdr, sizeof(phdr), 1, fp) != 1 ) {
			printf("read phdr wrong!\n");
		    exit(1);
		}
        //PT_LOAD 1
        if(phdr.p_type == 1 && phdr.p_memsz && phdr.p_filesz > 0){
            uint64_t prepad = phdr.p_vaddr % 4096; 
            uint64_t vaddr = phdr.p_vaddr - prepad;
            uint64_t postpad = 4096 - (phdr.p_vaddr + phdr.p_memsz) % 4096;
            uint64_t pmemsz = (phdr.p_vaddr + phdr.p_memsz + postpad) - vaddr;
            int prot = get_prot(phdr.p_flags);
            if(prot & PROT_WRITE) {
                data_seg.addr = vaddr;
                data_seg.size = pmemsz;
                continue;
            }
            if(prot & PROT_EXEC) {
                text_seg.addr = vaddr;
                text_seg.size = pmemsz;
                printf("text segment info, addr: 0x%lx, size: 0x%lx, endaddr: 0x%lx\n", phdr.p_vaddr, phdr.p_memsz, phdr.p_vaddr + phdr.p_memsz);
                printf("padded text segment info, addr: 0x%lx, size: 0x%lx, endaddr: 0x%lx\n", text_seg.addr, text_seg.size, vaddr + pmemsz);
            }	

            printf("--- load elf: alloc text memory: (0x%lx, 0x%lx) ---\n", text_seg.addr, text_seg.addr + text_seg.size);	

            //仅加载被使用到的代码段
            for(int i=0;i<numinfos;i++){
                fread(&textinfo, sizeof(MemRangeInfo), 1, fp1);
                unsigned int offset = textinfo.addr - phdr.p_vaddr + phdr.p_offset;
                fseek(fp, offset, SEEK_SET);
                printf("load text segment, addr: 0x%lx, size: 0x%lx, end: 0x%lx\n", textinfo.addr, textinfo.size, textinfo.addr + textinfo.size);
            }
        }
	}
    fclose(fp);
    fclose(fp1);
    return startvaddr;
}