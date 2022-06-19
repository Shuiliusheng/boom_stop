#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main()
{
	int a[100];
	for(int i=0;i<100;i+=20){
		for(int j=i;j<i+20;j++){
			a[j]=j;
			printf("%d ", j);
		}
		printf("\n");
	}
    printf("sp: 0x%lx\n", &a[0]);

	struct timeval tval;
    int ret = gettimeofday(&tval, NULL);
    if (ret == -1){
        printf("Error: gettimeofday()\n");
        return ret;
    }
	printf("start sec: %ld, usec: %ld\n", tval.tv_sec, tval.tv_usec);

	FILE *p=fopen("test.txt","wb");
	fwrite(&a[0], sizeof(int), 100, p);
	fclose(p);

	int *arr = (int *)malloc(sizeof(int)*2000);
	for(int i=0;i<2000;i++){
		arr[i]=a[i%50];
	}
    printf("alloc addr: 0x%lx\n", &arr[0]);

    FILE *p1=fopen("test.txt","rb");
	fread(&arr[0], sizeof(int), 100, p1);
	fclose(p1);

	struct timeval tval1;
    int ret1 = gettimeofday(&tval1, NULL);
    if (ret1 == -1){
        printf("Error: gettimeofday()\n");
        return ret1;
    }
	printf("end sec: %ld, usec: %ld\n", tval1.tv_sec, tval1.tv_usec);
    
	for(int i=0;i<100;i+=20){
		for(int j=i;j<i+20;j++){
			printf("%d ", arr[j]);
		}
		printf("\n");
	}
	return 0;
}
