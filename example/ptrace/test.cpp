#include "define.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

int arr[1000000];
uint64_t tag, eaddr, mevent, nevent, esel, mpriv=0;

void showinfo()
{
    GetInformation(tag, eaddr, mevent, nevent, esel, mpriv);
    printf("tag: 0x%lx, eaddr: 0x%lx, mevent: %d, nevent:%d, esel: %d, mpriv: %d\n", tag, eaddr, mevent, nevent, esel, mpriv);
}

int main()
{
    printf("start running\n");
    showinfo();
    for(int i=0;i<200;i++){
        for(int j=0;j<1000;j++){
            arr[rand()%1000000] = j+1;
        }
        printf("running place: %d\n", i);
        showinfo();
    }
    printf("sim end\n");
    return 0;
}