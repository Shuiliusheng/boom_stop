#include<stdio.h>
#include<stdlib.h>
#include<math.h>
int main()
{
	double arr[100];
	for(int i=0;i<1000;i++){
		double t = i * 1.0 / 0.3;
		arr[i%100] = 100/(t*t);
		printf("%d: %lf\n", i, arr[i%100]);
	}
	return 0;
}
