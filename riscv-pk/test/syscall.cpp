#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    //time
    struct timeval start;
    struct timeval end;
    if(gettimeofday(&start, NULL) == -1){
        printf("gettimeofday error\n");
        return 0;
    }
    printf("start, sec: %ld, usec: %ld\n", start.tv_sec, start.tv_usec);

    //malloc
    int *arr = (int *)malloc(sizeof(int)*2000);
    for(int i=0;i<2000;i++){
        arr[i]=2000-i;
    }

    //file op
    FILE *p=fopen("test.txt","wb");
	fwrite(&arr[0], sizeof(int), 2000, p);
	fclose(p);

    struct stat buf;
	int fd;
    fd = open("test.txt", O_RDONLY);
	fstat(fd, &buf);
   	printf("file size +%d\n ", buf.st_size);

    for(int i=0;i<2000;i++){
        arr[i]=i;
    }

    FILE *q=fopen("test.txt", "rb");
	fread(&arr[0], sizeof(int), 2000, q);
	fclose(q);
	
    if(gettimeofday(&end, NULL) == -1){
        printf("gettimeofday error\n");
        return 0;
    }
    printf("end, sec: %ld, usec: %ld\n", end.tv_sec, end.tv_usec);

    free(arr);
    return 0;
}