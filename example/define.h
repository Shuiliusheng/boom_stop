#ifndef   _DEFINE_H_  
#define   _DEFINE_H_


//-------------------------------------------------------------------
#define SetProcTag(srcreg)          "addi x0, " srcreg ", 1 \n\t"  
#define SetExitFuncAddr(srcreg)     "addi x0, " srcreg ", 2 \n\t"  
#define SetMaxInsts(srcreg)         "addi x0, " srcreg ", 3 \n\t"  
#define SetUScratch(srcreg)         "addi x0, " srcreg ", 4 \n\t" 
#define SetURetAddr(srcreg)         "addi x0, " srcreg ", 5 \n\t"  
#define SetMaxPriv(srcreg)          "addi x0, " srcreg ", 6 \n\t"  
#define SetTempReg(srcreg, rtemp)   "addi x0, " srcreg ", 7+" #rtemp " \n\t"  
#define SetStartInsts(srcreg)       "addi x0, " srcreg ", 10 \n\t"   

#define GetProcTag(dstreg)          "addi x0, " dstreg ", 1025 \n\t"  
#define GetUScratch(dstreg)         "addi x0, " dstreg ", 1026 \n\t"  
#define GetExitNPC(dstreg)          "addi x0, " dstreg ", 1027 \n\t"  
#define GetTempReg(dstreg, rtemp)   "addi x0, " dstreg ", 1028+" #rtemp " \n\t"  

#define URet() asm volatile( "addi x0, x0, 128  # uret \n\t" ); 

//---------------------------------------------------------------------

#define GetNPC(npc) asm volatile( \
    GetExitNPC("t0")    \
    "mv %[npc], t0  # uretaddr \n\t"  \
    : [npc]"=r"(npc)\
    :  \
); 

#define SetNPC(npc) asm volatile( \
    "mv t0, %[npc]    \n\t"  \
    SetURetAddr("t0")  \
    : \
    :[npc]"r"(npc)  \
); 

#define SetCounterLevel(priv) asm volatile( \
    "li t0, " priv "  # priv: user/super/machine \n\t"  \
    SetMaxPriv("t0")  \
); 


#define SetCtrlReg(tag, exitFucAddr, maxinst, startinst) asm volatile( \
    "mv t0, %[rtemp1]  # tag \n\t"  \
    SetProcTag("t0")        \
    "mv t0, %[rtemp2]  # exit \n\t"  \
    SetExitFuncAddr("t0")   \
    "mv t0, %[rtemp3]  # maxinst \n\t"  \
    SetMaxInsts("t0")       \
    "mv t0, %[rtemp4]  # startinst \n\t"  \
    SetStartInsts("t0")     \
    : \
    :[rtemp1]"r"(tag), [rtemp2]"r"(exitFucAddr), [rtemp3]"r"(maxinst), [rtemp4]"r"(startinst)\
); 


#define SetTempRegs(t1, t2, t3) asm volatile( \
    "mv t0, %[rtemp1]  \n\t"    \
    SetTempReg("t0", 0)       \
    "mv t0, %[rtemp2]  \n\t"    \
    SetTempReg("t0", 1)       \
    "mv t0, %[rtemp3]  \n\t"    \
    SetTempReg("t0", 2)       \
    : \
    :[rtemp1]"r"(t1), [rtemp2]"r"(t2), [rtemp3]"r"(t3)  \
);


#define Load_Basic_Regs() asm volatile( \
    GetTempReg("t0", 1)   \
    "ld sp,8*0(t0)  \n\t"   \
    "ld gp,8*1(t0)  \n\t"   \
    "ld tp,8*2(t0)  \n\t"   \
    "ld fp,8*3(t0)  \n\t"   \
    "ld ra,8*4(t0)  \n\t"   \
); 

#define Save_Basic_Regs() asm volatile( \
    GetTempReg("t0", 1)   \
    "sd gp,8*1(t0)  \n\t"   \
    "sd tp,8*2(t0)  \n\t"   \
    "sd fp,8*3(t0)  \n\t"   \
    "sd ra,8*4(t0)  \n\t"   \
); 

#define Save_ALLIntRegs() asm volatile( \
    SetUScratch("a0")       \
    GetTempReg("a0", 0)   \
    "sd x1,8*1(a0)  \n\t"   \
    "sd x2,8*2(a0)  \n\t"   \
    "sd x3,8*3(a0)  \n\t"   \
    "sd x4,8*4(a0)  \n\t"   \
    "sd x5,8*5(a0)  \n\t"   \
    "sd x6,8*6(a0)  \n\t"   \
    "sd x7,8*7(a0)  \n\t"   \
    "sd x8,8*8(a0)  \n\t"   \
    "sd x9,8*9(a0)  \n\t"   \
    "sd x10,8*10(a0)  \n\t"   \
    "sd x11,8*11(a0)  \n\t"   \
    "sd x12,8*12(a0)  \n\t"   \
    "sd x13,8*13(a0)  \n\t"   \
    "sd x14,8*14(a0)  \n\t"   \
    "sd x15,8*15(a0)  \n\t"   \
    "sd x16,8*16(a0)  \n\t"   \
    "sd x17,8*17(a0)  \n\t"   \
    "sd x18,8*18(a0)  \n\t"   \
    "sd x19,8*19(a0)  \n\t"   \
    "sd x20,8*20(a0)  \n\t"   \
    "sd x21,8*21(a0)  \n\t"   \
    "sd x22,8*22(a0)  \n\t"   \
    "sd x23,8*23(a0)  \n\t"   \
    "sd x24,8*24(a0)  \n\t"   \
    "sd x25,8*25(a0)  \n\t"   \
    "sd x26,8*26(a0)  \n\t"   \
    "sd x27,8*27(a0)  \n\t"   \
    "sd x28,8*28(a0)  \n\t"   \
    "sd x29,8*29(a0)  \n\t"   \
    "sd x30,8*30(a0)  \n\t"   \
    "sd x31,8*31(a0)  \n\t"   \
);  

#define Load_ALLIntRegs() asm volatile( \
    GetTempReg("a0", 0)   \
    "ld x1,8*1(a0)  \n\t"   \
    "ld x2,8*2(a0)  \n\t"   \
    "ld x3,8*3(a0)  \n\t"   \
    "ld x4,8*4(a0)  \n\t"   \
    "ld x5,8*5(a0)  \n\t"   \
    "ld x6,8*6(a0)  \n\t"   \
    "ld x7,8*7(a0)  \n\t"   \
    "ld x8,8*8(a0)  \n\t"   \
    "ld x9,8*9(a0)  \n\t"   \
    "ld x11,8*11(a0)  \n\t"   \
    "ld x12,8*12(a0)  \n\t"   \
    "ld x13,8*13(a0)  \n\t"   \
    "ld x14,8*14(a0)  \n\t"   \
    "ld x15,8*15(a0)  \n\t"   \
    "ld x16,8*16(a0)  \n\t"   \
    "ld x17,8*17(a0)  \n\t"   \
    "ld x18,8*18(a0)  \n\t"   \
    "ld x19,8*19(a0)  \n\t"   \
    "ld x20,8*20(a0)  \n\t"   \
    "ld x21,8*21(a0)  \n\t"   \
    "ld x22,8*22(a0)  \n\t"   \
    "ld x23,8*23(a0)  \n\t"   \
    "ld x24,8*24(a0)  \n\t"   \
    "ld x25,8*25(a0)  \n\t"   \
    "ld x26,8*26(a0)  \n\t"   \
    "ld x27,8*27(a0)  \n\t"   \
    "ld x28,8*28(a0)  \n\t"   \
    "ld x29,8*29(a0)  \n\t"   \
    "ld x30,8*30(a0)  \n\t"   \
    "ld x31,8*31(a0)  \n\t"   \
    GetUScratch("a0")         \
); 


//-------------------------------------------------------------------
#define RESET_COUNTER asm volatile(" andi x0, t0, 1024 \n\t" );

#define GetCounter(basereg, dstreg, start, n)    \
              "andi x0, " dstreg ", 512+" #start "+" #n " \n\t" \
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