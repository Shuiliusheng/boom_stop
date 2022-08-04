#include "define.h"
#include "perf.h"

uint64_t hpcounters[64];
uint64_t maxevent=0, warmupinst=0, eventsel = 0, maxperiods = 0;
uint64_t startcycle, endcycle, startinst=0, endinst=0;
FILE *logfile;

void event_sample_process(uint64_t sampletimes, uint64_t exitpc)
{
    endcycle = read_csr_cycle();
    endinst = read_csr_instret();

    fprintf(logfile, "{\"type\": \"max_inst\", \"times\": %d, \"cycles\": %ld, \"inst\": %ld}\n", sampletimes, endcycle - startcycle, endinst - startinst);
    printf("{\"type\": \"max_inst\", \"times\": %d, \"cycles\": %ld, \"inst\": %ld}\n", sampletimes, endcycle - startcycle, endinst - startinst);

    startcycle = endcycle;
    startinst = endinst;

    ReadCounter16(&hpcounters[0], 0);
    ReadCounter16(&hpcounters[16], 16);
    ReadCounter16(&hpcounters[32], 32);
    ReadCounter16(&hpcounters[48], 48);
	for(int n=0;n<64;n++){
		fprintf(logfile, "{\"type\": \"event %2d\", \"value\": %llu}\n", n, hpcounters[n]);
    }
}


int main(int argv, char **args)
{
    char **child_args = getargs(argv, args);
    if(child_args == NULL) 
        return 0;
    
    logfile=read_params(args[1], eventsel, maxevent, maxperiods, warmupinst);

    startcycle = read_csr_cycle();
    startinst = read_csr_instret();
    start_run_and_trace(args[2], child_args);
    return 0;
}