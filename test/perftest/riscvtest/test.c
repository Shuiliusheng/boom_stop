#include<stdio.h>
#include<stdlib.h>
#include"pfc_asm.h"

int arr[10240];
int test()
{
	//start testing .....
	for(int i=0;i<10;i++){
		for(int c=0;c<100;c++){
			arr[(i*c)%10240]=1;
		}
	}
	//end testing .....
	return 0;
}

int main()
{
	start_record();
	test();
	exit_record();
	return 0;
}
	
