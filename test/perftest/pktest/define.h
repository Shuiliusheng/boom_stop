#ifndef   _DEFINE_H_  
#define   _DEFINE_H_


//-------------------------------------------------------------------
#define SetProcTag(srcreg)          "addi x0, " srcreg ", 1 \n\t"  
#define SetExitFuncAddr(srcreg)     "addi x0, " srcreg ", 2 \n\t"  
#define SetMaxInsts(srcreg)         "addi x0, " srcreg ", 3 \n\t"  
#define SetUScratch(srcreg)         "addi x0, " srcreg ", 4 \n\t" 
#define SetURetAddr(srcreg)         "addi x0, " srcreg ", 5 \n\t"  
#define SetMaxPriv(srcreg)          "addi x0, " srcreg ", 6 \n\t"  
#define SetTempReg(srcreg, rtemp)   "addi x0, " srcreg ", 7+" rtemp " \n\t"  
#define SetStartInsts(srcreg)       "addi x0, " srcreg ", 10 \n\t"   

#define GetProcTag(dstreg)          "addi x0, " dstreg ", 1025 \n\t"  
#define GetUScratch(dstreg)         "addi x0, " dstreg ", 1026 \n\t"  
#define GetExitNPC(dstreg)          "addi x0, " dstreg ", 1027 \n\t"  
#define GetTempReg(dstreg, rtemp)   "addi x0, " dstreg ", 1028+" rtemp " \n\t"  

#define URet() asm volatile( "addi x0, x0, 128  # uret \n\t" ); 

#define GetPfcounter(dstreg, pfc)   "andi x0, " dstreg ", 32+" pfc " \n\t"
#define RESET_COUNTER asm volatile(" andi x0, t0, 64 \n\t" );
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
    SetTempReg("t0", "0")       \
    "mv t0, %[rtemp2]  \n\t"    \
    SetTempReg("t0", "1")       \
    "mv t0, %[rtemp3]  \n\t"    \
    SetTempReg("t0", "2")       \
    : \
    :[rtemp1]"r"(t1), [rtemp2]"r"(t2), [rtemp3]"r"(t3)  \
);


#define Load_necessary() asm volatile( \
    GetTempReg("t0", "1")   \
    "ld sp,8*0(t0)  \n\t"   \
    "ld gp,8*1(t0)  \n\t"   \
    "ld tp,8*2(t0)  \n\t"   \
    "ld fp,8*3(t0)  \n\t"   \
    "ld ra,8*4(t0)  \n\t"   \
); 

#define Save_necessary() asm volatile( \
    GetTempReg("t0", "1")   \
    "sd gp,8*1(t0)  \n\t"   \
    "sd tp,8*2(t0)  \n\t"   \
    "sd fp,8*3(t0)  \n\t"   \
    "sd ra,8*4(t0)  \n\t"   \
); 

#define Save_int_regs() asm volatile( \
    SetUScratch("a0")       \
    GetTempReg("a0", "0")   \
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

#define Load_int_regs() asm volatile( \
    GetTempReg("a0", "0")   \
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
//performance counter read event
unsigned long long read_counter(int n){
	unsigned long long temp=0;
	switch(n){
		case 0: asm volatile( GetPfcounter("t0", "0")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 1: asm volatile( GetPfcounter("t0", "1")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 2: asm volatile( GetPfcounter("t0", "2")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 3: asm volatile( GetPfcounter("t0", "3")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 4: asm volatile( GetPfcounter("t0", "4")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 5: asm volatile( GetPfcounter("t0", "5")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 6: asm volatile( GetPfcounter("t0", "6")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 7: asm volatile( GetPfcounter("t0", "7")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 8: asm volatile( GetPfcounter("t0", "8")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 9: asm volatile( GetPfcounter("t0", "9")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 10: asm volatile( GetPfcounter("t0", "10")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 11: asm volatile( GetPfcounter("t0", "11")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 12: asm volatile( GetPfcounter("t0", "12")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 13: asm volatile( GetPfcounter("t0", "13")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 14: asm volatile( GetPfcounter("t0", "14")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 15: asm volatile( GetPfcounter("t0", "15")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
                                                                        
        case 16: asm volatile( GetPfcounter("t0", "16")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 17: asm volatile( GetPfcounter("t0", "17")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 18: asm volatile( GetPfcounter("t0", "18")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 19: asm volatile( GetPfcounter("t0", "19")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 20: asm volatile( GetPfcounter("t0", "20")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 21: asm volatile( GetPfcounter("t0", "21")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 22: asm volatile( GetPfcounter("t0", "22")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 23: asm volatile( GetPfcounter("t0", "23")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 24: asm volatile( GetPfcounter("t0", "24")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 25: asm volatile( GetPfcounter("t0", "25")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 26: asm volatile( GetPfcounter("t0", "26")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 27: asm volatile( GetPfcounter("t0", "27")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 28: asm volatile( GetPfcounter("t0", "28")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 29: asm volatile( GetPfcounter("t0", "29")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 30: asm volatile( GetPfcounter("t0", "30")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
        case 31: asm volatile( GetPfcounter("t0", "31")  " mv %[out], t0   \n\t" :[out]"=r"(temp) : ); break;
		default: break;
	}
	return temp;
}


//-------------------------------------------------------------------
#define DEFINE_CSRR(s)                     \
    static inline unsigned long long __csrr_##s()    \
    {                                      \
        unsigned long long value;                    \
        __asm__ volatile("csrr    %0, " #s \
                         : "=r"(value)     \
                         :);               \
        return value;                      \
    }

DEFINE_CSRR(cycle)
DEFINE_CSRR(instret)

#endif