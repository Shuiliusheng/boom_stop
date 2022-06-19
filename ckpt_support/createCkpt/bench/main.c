#include<stdio.h>
#include<stdlib.h>
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

	FILE *q=fopen("test.txt", "rb");
	int val=0;
	fscanf(q,"%d", &val);
	fread(&a[0], 4, 10, q);
	fclose(q);
	printf("%d %d %d\n", a[1], a[2], val);

	int *arr;
	arr = (int *)malloc(sizeof(int)*1024*1024*8);
	for(int i=0;i<1024;i++){
		arr[i*1024*8] = i;
	}
	free(arr);

	return 0;
}
