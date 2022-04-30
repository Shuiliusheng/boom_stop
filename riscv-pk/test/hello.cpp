#include<stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

int gp[10];

int main()
{
    int sp[10];
    printf("hello world\n");
    printf("gp: 0x%lx, sp: 0x%lx\n", &gp[0], &sp[0]);

    uint64_t alloc_vaddr = (uint64_t)mmap((void*)0x150000, 4096*4096, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_FIXED, -1, 0);
    uint64_t alloc_vaddr1 = (uint64_t)mmap((void*)0x150000, 4096*4096, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_FIXED, -1, 0);
    printf("cannot alloc memory in 0x%lx for record runtime information, alloc: 0x%lx\n", alloc_vaddr1, alloc_vaddr);

    return 0;
}