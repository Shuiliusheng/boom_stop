#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/utsname.h>

int main()
{
	int a[10];
	for(int i=0;i<10;i++){
		a[i]=i;
		printf("%d\n", a[i]);
	}
	puts("hello\n");

	FILE *p=fopen("test.txt","wb");
	fprintf(p, "12345\n");
	fwrite(&a[0], sizeof(int), 10, p);
	fclose(p);

	a[1]=10;
	a[2]=11;

	struct utsname testbuff;
	printf("testbuff addr: 0x%lx\n", &testbuff);
	if( uname(&testbuff) >=0 )
		printf(" sysname:%s\n nodename:%s\n release:%s\n version:%s\n machine:%s\n \n ", testbuff.sysname, testbuff.nodename, testbuff.release,testbuff.version, testbuff.machine);


	struct stat buf;
	int fd;
    fd = open("test.txt", O_RDONLY);
	fstat(fd, &buf);
   	printf("file size +%d\n ", buf.st_size);
	printf("buf addr: 0x%lx, %d\n", (unsigned long long)&buf, sizeof(struct stat));

	printf("stat, dev: 0x%lx, ino: 0x%lx, mode: 0x%lx\n", buf.st_dev, buf.st_ino, buf.st_mode);
	printf("stat, nlink: 0x%lx, uid: 0x%lx, gid: 0x%lx, rdev: 0x%lx\n", buf.st_nlink, buf.st_uid, buf.st_gid, buf.st_rdev);
	printf("stat, size: 0x%lx, blksize: 0x%lx, blocks: 0x%x\n", buf.st_size, buf.st_blksize, buf.st_blocks);
	

	char path[255];
	printf("%s\n", getcwd(path, sizeof(path)));


	struct timeval tval;
    int ret = gettimeofday(&tval, NULL);
    if (ret == -1){
        printf("Error: gettimeofday()\n");
        return ret;
    }
    printf("sec: %ld, usec: %ld\n", tval.tv_sec, tval.tv_usec);

	FILE *q=fopen("test.txt", "rb");
	int val=0;
	fscanf(q,"%d", &val);
	fread(&a[0], 4, 10, q);
	fclose(q);
	printf("%d %d %d\n", a[1], a[2], val);

	return 0;
}
