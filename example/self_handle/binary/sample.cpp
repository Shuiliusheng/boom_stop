#include <stdlib.h>
#include <fcntl.h>
#include "ctrl.h"

uint64_t maxevent = 100, eventsel = 0, warmupinst = 0, maxperiod = 0;
uint64_t startcycle, endcycle, startinst, endinst;
uint64_t hpcounters[64];
unsigned int logfile;

void sample_func(uint64_t sampletimes, uint64_t exitpc)
{
    char str[300];
    endcycle = read_csr_cycle();
    endinst = read_csr_instret();

    sprintf(str, "{\"type\": \"max_inst\", \"times\": %d, \"cycles\": %ld, \"inst\": %ld}\n", sampletimes, endcycle - startcycle, endinst - startinst);
    write(1, str, strlen(str));
    if(logfile!=1) write(logfile, str, strlen(str));

    startcycle = endcycle;
    startinst = endinst;

    ReadCounter16(&hpcounters[0], 0);
    ReadCounter16(&hpcounters[16], 16);
    ReadCounter16(&hpcounters[32], 32);
    ReadCounter16(&hpcounters[48], 48);
	for(int n=0;n<64;n++){
		sprintf(str, "{\"type\": \"event %2d\", \"value\": %llu}\n", n, hpcounters[n]);
        write(logfile, str, strlen(str));
    }
}


bool findidx(char src[], const char dst[], int &idx)
{
    int l1=strlen(src), l2 = strlen(dst);
    if(l1 <= l2+1) return false;
    char c = src[l2];
    src[l2] = '\0';
    if(strcmp(src, dst)==0){
        for(int i=l2+1;i<l1;i++){
            if(src[i]==' '||src[i]==':')
                continue;
            else{
                idx = i;
                break;
            }
        }
        src[l2] = c;
        return true;
    }
    src[l2] = c;
    return false;
}

bool read_params(const char filename[])
{
    FILE *p=fopen(filename,"r");
    if(p==NULL) {
        printf("%s cannot be opened\n", filename);
        if(access("counter.log", R_OK) != -1){
            printf("counter.log file is exist\n");
            exit(0);
        }
        logfile=open("counter.log",O_RDWR | O_CREAT);
        printf("logname: counter.log\n");
        return false;
    }

    char str[100];
    int idx = 0;
    while(!feof(p)) {
        fgets(str, 100, p);
        if(strlen(str) < 1) break;
        if(str[strlen(str)-1] == '\n')
            str[strlen(str)-1] = '\0';

        if(findidx(str, "maxevent", idx)) {
            sscanf(&str[idx], "%lld", &maxevent);
        }
        else if(findidx(str, "eventsel", idx)) {
            sscanf(&str[idx], "%d", &eventsel);
        }
        else if(findidx(str, "maxperiod", idx)) {
            sscanf(&str[idx], "%lld", &maxperiod);
        }
        else if(findidx(str, "warmupinst", idx)) {
            sscanf(&str[idx], "%lld", &warmupinst);
        }
        else if(findidx(str, "logname", idx)) {
            printf("logname: %s\n", &str[idx]);
            if(access(&str[idx], R_OK) == -1)
                logfile=open(&str[idx], O_RDWR | O_CREAT);
            else
                logfile=open(&str[idx], O_RDWR);
            if(logfile == -1) {
                printf("cannot open %s to write\n", &str[idx]);
                exit(0);
            }
        }
        strcpy(str, "");
    }
    fclose(p);
    return true;
}
