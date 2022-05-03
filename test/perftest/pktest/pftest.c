#include<stdio.h>
#include<stdlib.h>
#include"ctrl.h"

int test()
{
	//start testing .....
	for(int i=0;i<30;i++){
		int *arr = (int *)malloc(sizeof(int)*1024);
		for(int c=0;c<1024;c++)
			for(int j=0;j<104;j++){
			int c=rand()%5;
			if(c>2)
				arr[(c+j)%1024]=1;
		}
	}
	//end testing .....
	return 0;
}

int main()
{
	test();
	return 0;
}
	
