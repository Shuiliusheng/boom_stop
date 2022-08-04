#ifndef   _DEFINE_H_  
#define   _DEFINE_H_


//-------------------------------------------------------------------
//设置和读取一些新增加的特殊寄存器值
#define Set_ProcTag(srcreg)          "addi x0, " srcreg ", 1 \n\t"  
#define Set_SampleFuncAddr(srcreg)   "addi x0, " srcreg ", 2 \n\t"  
#define Set_EventSel(srcreg)         "addi x0, " srcreg ", 3 \n\t"   
#define Set_MaxEvents(srcreg)        "addi x0, " srcreg ", 4 \n\t"  
#define Set_WarmupInsts(srcreg)      "addi x0, " srcreg ", 5 \n\t"   
#define Set_MaxPriv(srcreg)          "addi x0, " srcreg ", 6 \n\t"  
#define Set_PfcEnable(srcreg)        "addi x0, " srcreg ", 7 \n\t"   
#define Set_SampleHap(srcreg)        "addi x0, " srcreg ", 8 \n\t"   
#define Set_TempReg(srcreg, rtemp)   "addi x0, " srcreg ", 9+" #rtemp " \n\t"  

#define Get_ProcTag(dstreg)         "addi x0, " dstreg ", 33 \n\t" 
#define Get_SampleHap(dstreg)       "addi x0, " dstreg ", 34 \n\t"  
#define Get_ExitNPC(dstreg)         "addi x0, " dstreg ", 35 \n\t"  
#define Get_TempReg(dstreg, rtemp)  "addi x0, " dstreg ", 36+" #rtemp " \n\t"  
#define Get_SampleFuncAddr(dstreg)  "addi x0, " dstreg ", 41 \n\t"  
#define Get_MaxEvents(dstreg)       "addi x0, " dstreg ", 42 \n\t"  
#define Get_NowEvents(dstreg)       "addi x0, " dstreg ", 43 \n\t"  
#define Get_EventSel(dstreg)        "addi x0, " dstreg ", 44 \n\t"  
#define Get_Maxpriv(dstreg)         "addi x0, " dstreg ", 45 \n\t"  

#define SpecialInst_URet(rtemp)     "addi x0, x" rtemp ", 64 \n\t" 
#define SpecialInst_RstPFC          "addi x0, x0, 127 \n\t" 

//---------------------------------------------------------------------

#define GetInformation(tag, eaddr, mevent, nevent, esel, mpriv) asm volatile( \
    Get_ProcTag("t0")    \
    "mv %[v0], t0  \n\t"  \
    Get_SampleFuncAddr("t0")    \
    "mv %[v1], t0  \n\t"  \
    Get_MaxEvents("t0")    \
    "mv %[v2], t0  \n\t"  \
    Get_NowEvents("t0")    \
    "mv %[v3], t0  \n\t"  \
    Get_EventSel("t0")    \
    "mv %[v4], t0  \n\t"  \
    Get_Maxpriv("t0")    \
    "mv %[v5], t0  \n\t"  \
    : [v0]"=r"(tag), [v1]"=r"(eaddr), [v2]"=r"(mevent), [v3]"=r"(nevent), [v4]"=r"(esel), [v5]"=r"(mpriv)\
    :  \
); 


#define GetExitPC(npc) asm volatile( \
    Get_ExitNPC("t0")    \
    "mv %[value], t0  # uretaddr \n\t"  \
    : [value]"=r"(npc)\
    :  \
); 

#define GetSampleHappen(shap) asm volatile( \
    Get_SampleHap("t0") \
    "mv %[value], t0   \n\t"  \
    : [value]"=r"(shap)\
    :  \
); 

#define SetSampleHappen(shap) asm volatile( \
    "mv t0, %[value]    \n\t"  \
    Set_SampleHap("t0")  \
    : \
    :[value]"r"(shap)  \
); 


#define SetCounterLevel(priv) asm volatile( \
    "li t0, " priv "  # priv: user/super/machine \n\t"  \
    Set_MaxPriv("t0")  \
); 


#define SetTempReg(value, n) asm volatile( \
    "mv t0, %[rtemp1]  \n\t"    \
    Set_TempReg("t0", n)         \
    : \
    :[rtemp1]"r"(value)         \
);


#define SetPfcEnable(n) asm volatile( \
    "li t0, " #n "  \n\t"  \
    Set_PfcEnable("t0")  \
); 


#define JmpTempReg(n) asm volatile( \
    SpecialInst_URet(#n) \
);


#define SetSampleBaseInfo(ptag, sampleFuncAddr) asm volatile( \
    "mv t0, %[value1]   \n\t"  \
    Set_ProcTag("t0")        \
    "mv t0, %[value2]   \n\t"  \
    Set_SampleFuncAddr("t0")   \
    : \
    :[value1]"r"(ptag), [value2]"r"(sampleFuncAddr)  \
); 


#define SetSampleCtrlReg(maxevent, warmupinst, eventsel) asm volatile( \
    "mv t0, %[value1]  # maxinst \n\t"  \
    Set_MaxEvents("t0")       \
    "mv t0, %[value2]  # warmup \n\t"  \
    Set_WarmupInsts("t0")     \
    "mv t0, %[value3]  # evensel \n\t"  \
    Set_EventSel("t0")       \
    "li t0, 1    \n\t"      \
    "addi x0, t0, 12  #SetPfcEnable \n\t"  \
    : \
    :[value1]"r"(maxevent), [value2]"r"(warmupinst), [value3]"r"(eventsel) \
); 


extern uint64_t tempStackMem[4096];
#define Load_Basic_Regs() asm volatile( \
    "la t0, tempStackMem  \n\t"   \
    "addi t0, t0, 256  \n\t"   \
    "ld sp,8*0(t0)  \n\t"   \
    "ld s0,8*1(t0)  \n\t"   \
    "ld ra,8*2(t0)  \n\t"   \
    "ld gp,8*3(t0)  \n\t"   \
    "ld tp,8*4(t0)  \n\t"   \
); 

#define Save_Basic_Regs() asm volatile( \
    "la t0, tempStackMem  \n\t"   \
    "addi t0, t0, 256  \n\t"   \
    "sd s0,8*1(t0)  \n\t"   \
    "sd ra,8*2(t0)  \n\t"   \
    "sd gp,8*3(t0)  \n\t"   \
    "sd tp,8*4(t0)  \n\t"   \
); 

#define Regs_Operations(Op) \
    Op "1,8*1(a0)  \n\t"   \
    Op "2,8*2(a0)  \n\t"   \
    Op "3,8*3(a0)  \n\t"   \
    Op "4,8*4(a0)  \n\t"   \
    Op "5,8*5(a0)  \n\t"   \
    Op "6,8*6(a0)  \n\t"   \
    Op "7,8*7(a0)  \n\t"   \
    Op "8,8*8(a0)  \n\t"   \
    Op "9,8*9(a0)  \n\t"   \
    Op "11,8*11(a0)  \n\t"   \
    Op "12,8*12(a0)  \n\t"   \
    Op "13,8*13(a0)  \n\t"   \
    Op "14,8*14(a0)  \n\t"   \
    Op "15,8*15(a0)  \n\t"   \
    Op "16,8*16(a0)  \n\t"   \
    Op "17,8*17(a0)  \n\t"   \
    Op "18,8*18(a0)  \n\t"   \
    Op "19,8*19(a0)  \n\t"   \
    Op "20,8*20(a0)  \n\t"   \
    Op "21,8*21(a0)  \n\t"   \
    Op "22,8*22(a0)  \n\t"   \
    Op "23,8*23(a0)  \n\t"   \
    Op "24,8*24(a0)  \n\t"   \
    Op "25,8*25(a0)  \n\t"   \
    Op "26,8*26(a0)  \n\t"   \
    Op "27,8*27(a0)  \n\t"   \
    Op "28,8*28(a0)  \n\t"   \
    Op "29,8*29(a0)  \n\t"   \
    Op "30,8*30(a0)  \n\t"   \
    Op "31,8*31(a0)  \n\t"   


#define Save_ALLIntRegs() asm volatile( \
    Set_TempReg("a0", 0)  \
    "la a0, tempStackMem  \n\t"   \
    Regs_Operations("sd x") \
    Get_TempReg("a1", 0)  \
    "sd a1,8*10(a0)  # save a0 \n\t"   \
);  

#define Load_ALLIntRegs() asm volatile( \
    "la a0, tempStackMem  \n\t"    \
    Regs_Operations("ld x") \
    "ld a0,8*10(a0) # restore a0 \n\t"   \
); 


//-------------------------------------------------------------------
#define RESET_COUNTER asm volatile(SpecialInst_RstPFC);

#define GetCounter(basereg, dstreg, start, n)    \
              "addi x0, " dstreg ", 128+" #start "+" #n " \n\t" \
              "sd " dstreg ", " #n "*8(" basereg ") \n\t"

#define ReadCounter8(base, start) asm volatile( \
    "mv t1, %[addr]  # set base addr \n\t"  \
    GetCounter("t1", "t0", start, 0)  \
    GetCounter("t1", "t0", start, 1)  \
    GetCounter("t1", "t0", start, 2)  \
    GetCounter("t1", "t0", start, 3)  \
    GetCounter("t1", "t0", start, 4)  \
    GetCounter("t1", "t0", start, 5)  \
    GetCounter("t1", "t0", start, 6)  \
    GetCounter("t1", "t0", start, 7)  \
    : \
    :[addr]"r"(base) \
); 

#define ReadCounter16(base, start) \
    ReadCounter8(base, start) \
    ReadCounter8(base+8, start+8) 


//-------------------------------------------------------------------
#define DEF_CSRR(s)                     \
    static inline unsigned long long read_csr_##s()    \
    {                                      \
        unsigned long long value;                    \
        __asm__ volatile("csrr    %0, " #s \
                         : "=r"(value)     \
                         :);               \
        return value;                      \
    }

DEF_CSRR(cycle)
DEF_CSRR(instret)

#endif