#include "perf.h"

char **getargs(int argv, char **args)
{
    char **res = NULL;
    if(argv<4){
        printf("parameters are not enough!\n");
        printf("./ptrace paramfile pathname filename args\n");
        return res;
    }

    char *pathname = args[2];
    if (access(pathname, F_OK) != 0) {
        printf("%s is not exist!\n", pathname);
        return NULL;
    }

    int maxlen = 0;
    for(int i=2;i<argv;i++){
        if(maxlen < strlen(args[i])) {
            maxlen = strlen(args[i]);
        }
    }
    argv = argv - 2;
    res = (char **)malloc(sizeof(char *)*argv);
    for(int i=0;i<argv-1;i++){
        res[i] = (char *)malloc(sizeof(char)*maxlen);
        strcpy(res[i], args[i+3]);
        printf("arg %d: %s\n", i, res[i]);
    }
    res[argv-1] = NULL;
    return res;
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

FILE *read_params(const char filename[], uint64_t &eventsel, uint64_t &maxevent, uint64_t &maxperiods, uint64_t &warmupinst)
{
    FILE *p=fopen(filename,"r");
    if(p==NULL) {
        printf("%s cannot be opened\n", filename);
        exit(0);
    }

    char str[256];
    char logname[256];
    int idx = 0;
    while(!feof(p)) {
        fgets(str, 100, p);
        if(strlen(str) < 1) 
            break;
        if(str[strlen(str)-1] == '\n')
            str[strlen(str)-1] = '\0';

        if(findidx(str, "eventsel", idx)) {
            sscanf(&str[idx], "%d", &eventsel);
        }
        else if(findidx(str, "maxevent", idx)) {
            sscanf(&str[idx], "%lld", &maxevent);
        }
        else if(findidx(str, "maxperiod", idx)) {
            sscanf(&str[idx], "%lld", &maxperiods);
        }
        else if(findidx(str, "warmupinst", idx)) {
            sscanf(&str[idx], "%lld", &warmupinst);
        }
        else if(findidx(str, "logname", idx)) {
            strcpy(logname, &str[idx]);
        }
        strcpy(str, "");
    }
    fclose(p);

    printf("eventsel: %d\n", eventsel);
    printf("maxevent: %d\n", maxevent);
    printf("maxperiod: %d\n", maxperiods);
    printf("warmupinst: %d\n", warmupinst);
    printf("logname: %s\n", logname);
    FILE *log = fopen(logname, "w");
    if(log == NULL) {
        printf("%s cannot be write\n", logname);
        log=stdout;
    }
    return log;
}
