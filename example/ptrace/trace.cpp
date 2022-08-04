#include "define.h"
#include "perf.h"
#define PERF_PROTAG 0x1234567

void start_run_and_trace(char pathname[], char **args)
{
    pid_t child;
    int status;
    child = fork();
    if(child == 0) {
        //setup the sampled program
        uint64_t procTag = PERF_PROTAG;
        uint64_t sampleFuncAddr = 0;
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        //set the basic sample information
        SetSampleBaseInfo(procTag, sampleFuncAddr);
        uint64_t maxevent = 0, warmupinst = 0, eventsel = 0;
        SetSampleCtrlReg(maxevent, warmupinst, eventsel);
        execv(pathname, args);
    }

    struct user_regs_struct regs;
    struct iovec iov;
    iov.iov_len = sizeof(regs);
    iov.iov_base = &regs;

    uint64_t sampleHappen = 0;
    uint64_t oldcallnum = 0;
    uint64_t sampletimes = 0;
    uint64_t runtimes = 0;
    waitpid(child, &status, 0); //等待并记录execve
    if(WIFEXITED(status))
        return ;
    while(1) {
        //track the syscall of children process
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        waitpid(child, &status, 0);
        if(WIFEXITED(status))
            break;
        
        //read all register and pc of children process
        ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov);

        //whether this syscall is caused by a sample event
        GetSampleHappen(sampleHappen);
        if(sampleHappen != 0) {
            //reset the sampleHappen register to zero
            sampleHappen = 0;
            SetSampleHappen(sampleHappen);
            //record the syscall num and set it to a undefined syscall
            oldcallnum = regs.a7;
            regs.a7 = 3656;
            //set the pc = pc - 4, since the syscall will ret to pc + 4
            regs.pc = regs.pc - 4;
            //set the registers of children process with new value
            ptrace(PTRACE_SETREGSET, child, NT_PRSTATUS, &iov);
            //sampling function
            printf("---- sample: %d, pc : 0x%lx\n", sampletimes, regs.pc+4);
            event_sample_process(sampletimes, regs.pc+4);
            sampletimes++;
            if(sampletimes > maxperiods)
                break;
        }

        ptrace(PTRACE_SYSCALL, child, 0, 0);
        waitpid(child, &status, 0);
        if(WIFEXITED(status))
            break;

        //after the syscall processing over
        if(regs.a7 == 3656) {
            //if the sycall is caused by sampling, recovery the a7 register
            regs.a7 = oldcallnum;
            ptrace(PTRACE_SETREGSET, child, NT_PRSTATUS, &iov);
            //set the next event sample setting
            SetPfcEnable(1);
            SetSampleCtrlReg(maxevent, warmupinst, eventsel);
            RESET_COUNTER;
        }
        else if(regs.a7 == 214 && runtimes == 0){ //brk
            printf("start sampling program (the first brk)\n");
            //set the first event sample setting
            SetPfcEnable(1);
            SetSampleCtrlReg(maxevent, warmupinst, eventsel);
            RESET_COUNTER;
            warmupinst = 0;
            runtimes = 1;
        }
    }
    kill(child,SIGKILL);
    printf("sample over\n");
}
