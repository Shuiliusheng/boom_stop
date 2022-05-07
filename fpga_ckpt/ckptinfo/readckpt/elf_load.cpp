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

//replace ecall with jmptemp 6
void replaceEcall(uint16_t *text, uint64_t length)
{
    uint16_t data1 = 0x0000, data0 = 0x0073;
    //JmpTemp("1");//00106033
    uint16_t pdata1 = (ECall_Replace) >> 16, pdata0 = (ECall_Replace)%65536;
    for(int i=0;i<length;i++) {
        if(text[i] == data0 && text[i+1] == data1){
            text[i] = pdata0;
            text[i+1] = pdata1;
        }
    }
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
    if (fp1 == NULL) {
		printf("cannot open %s\n", progname);
        exit(1);
	}
    MemRangeInfo textinfo;
    uint64_t numinfos = 0;
    fread(&numinfos, 8, 1, fp1);
    printf("textinfo: %d\n", numinfos);

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
                if(ShowLog){
                    printf("text segment info, addr: 0x%lx, size: 0x%lx, endaddr: 0x%lx\n", phdr.p_vaddr, phdr.p_memsz, phdr.p_vaddr + phdr.p_memsz);
                    printf("padded text segment info, addr: 0x%lx, size: 0x%lx, endaddr: 0x%lx\n", text_seg.addr, text_seg.size, vaddr + pmemsz);
                }
            }	

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
                // printf("load text segment, addr: 0x%lx, size: 0x%lx, end: 0x%lx\n", textinfo.addr, textinfo.size, textinfo.addr + textinfo.size);
                fread((char *)textinfo.addr, textinfo.size, 1, fp); 
                replaceEcall((uint16_t *)textinfo.addr, textinfo.size/2);
            }

            if(numinfos==0) {
                //find text segment and replace ecall with jmp rtemp
                Elf64_Shdr string_shdr, shdr;
                fseek(fp, ehdr.e_shoff + ehdr.e_shstrndx * ehdr.e_shentsize, SEEK_SET);
                fread(&string_shdr, sizeof(string_shdr), 1, fp);
                fseek(fp, string_shdr.sh_offset + string_shdr.sh_size, SEEK_SET);
                printf("--- ecall replace in text segment --- \n");
                for (int c = 0; c < ehdr.e_shnum; c++) {
                    fseek(fp, ehdr.e_shoff + c * ehdr.e_shentsize, SEEK_SET);
                    fread(&shdr, sizeof(shdr), 1, fp);
                    
                    if(shdr.sh_size!=0 && shdr.sh_flags & 0x4){//SHF_EXECINSTR = 0x4
                        fseek(fp, shdr.sh_offset, SEEK_SET);
                        if(ShowLog){
                            printf("load executable segment, addr: 0x%lx, size: 0x%lx, end: 0x%lx\n", shdr.sh_addr, shdr.sh_size, shdr.sh_size+shdr.sh_addr);
                        }
                        //选择只load可以被执行的代码段，其余不加载，以减少恢复时间
                        if (fread((void *)shdr.sh_addr, shdr.sh_size, 1, fp) != 1) {    //加载执行的段
                            printf("cannot read shdr from file\n");
                            exit(1);
                        }
                        replaceEcall((uint16_t *)shdr.sh_addr, shdr.sh_size/2);
                    }
                }
                // if (!(prot & PROT_WRITE)){
                //     if (mprotect((void *)phdr.p_vaddr, phdr.p_memsz, prot) == -1) {
                //         printf("mprotect error: %d\n", i);
                //         exit(1);
                //     }
                // }
            }
        }
	}
    fclose(fp);
    fclose(fp1);
    cycles[1] = __csrr_cycle();
    insts[1] = __csrr_instret();

    printf("load elf running info, cycles: %ld, insts: %ld\n", cycles[1]-cycles[0], insts[1]-insts[0]);
    return startvaddr;
}