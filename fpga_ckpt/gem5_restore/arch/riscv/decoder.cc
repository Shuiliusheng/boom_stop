/*
 * Copyright (c) 2012 Google
 * Copyright (c) The University of Virginia
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "arch/riscv/decoder.hh"
#include "arch/riscv/types.hh"
#include "base/bitfield.hh"
#include "debug/Decode.hh"
#include "debug/TempInst.hh"

namespace gem5
{

namespace RiscvISA
{

void Decoder::reset()
{
    aligned = true;
    mid = false;
    more = true;
    emi = 0;
    instDone = false;
}

void
Decoder::moreBytes(const PCState &pc, Addr fetchPC)
{
    // The MSB of the upper and lower halves of a machine instruction.
    constexpr size_t max_bit = sizeof(machInst) * 8 - 1;
    constexpr size_t mid_bit = sizeof(machInst) * 4 - 1;

    auto inst = letoh(machInst);
    DPRINTF(Decode, "Requesting bytes 0x%08x from address %#x\n", inst,
            fetchPC);

    bool aligned = pc.pc() % sizeof(machInst) == 0;
    if (aligned) {
        emi = inst;
        if (compressed(emi))
            emi = bits(emi, mid_bit, 0);
        more = !compressed(emi);
        instDone = true;
    } else {
        if (mid) {
            assert(bits(emi, max_bit, mid_bit + 1) == 0);
            replaceBits(emi, max_bit, mid_bit + 1, inst);
            mid = false;
            more = false;
            instDone = true;
        } else {
            emi = bits(inst, max_bit, mid_bit + 1);
            mid = !compressed(emi);
            more = true;
            instDone = compressed(emi);
        }
    }
}

StaticInstPtr
Decoder::decode(ExtMachInst mach_inst, Addr addr)
{
    DPRINTF(Decode, "Decoding instruction 0x%08x at address %#x\n",
            mach_inst, addr);

    //and x0, rd, rtemp: read temp,     op = 0x33, func7 = 0, func3 = 7
    //sll x0, rs1, rtemp: write temp,   op = 0x33, func7 = 0, func3 = 1
    //xor x0, x0, rtemp: jmp rtemp,      op = 0x33, func7 = 0, func3 = 4

    unsigned int taddinst = 0x80b3; //add rd (1), rs1(1), rs2(0)
    unsigned int tjalrinst = 0x8067; //jalr rd(0), rs1(1), 0
    ExtMachInst old_mach_inst = mach_inst;

    unsigned int opcode = 0, rs1 = 0, rs2 = 0, rd = 1, func7 = 0, func3 = 0;
    bool isRTemp = false, isWTemp = false, isJTemp = false;
    if (!compressed(emi)) {
        opcode = mach_inst % 128;
        func7 = mach_inst >> 25;
        func3 = (mach_inst >> 12) % 8;
        rs1 = (mach_inst >> 15) % 32;
        rs2 = (mach_inst >> 20) % 32;
        rd = (mach_inst >> 7) % 32;

        isRTemp = (opcode == 0x33) && (func7 == 0)  && (func3 == 7) && (rd == 0);
        isWTemp = (opcode == 0x33) && (func7 == 0) && (func3 == 1) && (rd == 0);
        isJTemp = (opcode == 0x33) && (func7 == 0)  && (func3 == 4) && (rd == 0);
    }
    
    if(isRTemp || isWTemp){
        mach_inst = taddinst;
    }
    else if(isJTemp){
        mach_inst = tjalrinst;
    }

    StaticInstPtr &si = instMap[mach_inst];
    if (!si || isRTemp || isWTemp || isJTemp){ //如果是临时指令，则不从cache中取，而是直接重新译码
        si = decodeInst(mach_inst);
    }  

    if(isRTemp) {
        si->setSrcRegIdx(0, RegId(IntRegClass, rs2+32));
        si->setDestRegIdx(0, RegId(IntRegClass, rs1));
    }
    else if(isWTemp) {
        si->setSrcRegIdx(0, RegId(IntRegClass, rs1));
        si->setDestRegIdx(0, RegId(IntRegClass, rs2+32));
    }
    else if(isJTemp) {
        si->setSrcRegIdx(0, RegId(IntRegClass, rs2+32));
    }

    if(isRTemp || isWTemp || isJTemp){
        StaticInstPtr tsi = decodeInst(old_mach_inst);
        DPRINTF(TempInst, "Decode: Temp Inst, addr: 0x%lx, inst: 0x%lx, %s, R: %d, W: %d, J: %d, rs1: %d, rs2: %d\n", addr, old_mach_inst, tsi->disassemble(addr).c_str(), isRTemp, isWTemp, isJTemp, rs1, rs2);
        DPRINTF(TempInst, "Decode: Decoded Temp %s instruction: %#x, %s\n", si->getName(), mach_inst, si->disassemble(addr).c_str());
    }

    DPRINTF(Decode, "Decode: Decoded %s instruction: %#x\n",
            si->getName(), mach_inst);
    return si;
}

StaticInstPtr
Decoder::decode(RiscvISA::PCState &nextPC)
{
    if (!instDone)
        return nullptr;
    instDone = false;

    if (compressed(emi)) {
        nextPC.npc(nextPC.instAddr() + sizeof(machInst) / 2);
    } else {
        nextPC.npc(nextPC.instAddr() + sizeof(machInst));
    }

    return decode(emi, nextPC.instAddr());
}

} // namespace RiscvISA
} // namespace gem5
