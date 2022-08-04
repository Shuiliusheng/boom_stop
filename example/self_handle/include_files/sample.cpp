#include "ctrl.h"

int arr[1000000];
int main()
{
    printf("start running\n");
    // showInfo();
    for(int i=0;i<200;i++){
        for(int j=0;j<1000;j++){
            arr[rand()%1000000] = j+1;
        }
        printf("running place: %d\n", i);
    }
    printf("sim end\n");
    return 0;
}