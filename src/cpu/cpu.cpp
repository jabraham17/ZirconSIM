#include "cpu.h"
#include "isa/inst.h"
#include "isa/instruction_match.h"
#include "isa/rf.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace cpu {

// use raw(addr) so we don't log mem access
uint32_t HartState::getInstWord() { return *((uint32_t*)memimg.raw(pc)); }

HartState::HartState(mem::MemoryImage& m, TraceMode tm) : memimg(m) {
    rf.setTraceMode(tm);
}

Hart::Hart(mem::MemoryImage& m, TraceMode tm)
    : hs(m, tm), trace_mode(tm), trace_inst("INSTRUCTION TRACE") {
    trace_inst.setState((trace_mode & TraceMode::INSTRUCTION));
}

bool Hart::shouldHalt() {
    // if the current inst is a jump to itself, halt
    uint32_t inst = hs.getInstWord();
    auto op = isa::inst::decodeInstruction(inst);
    auto jmp_target =
        instruction::signext64<20>(instruction::getJTypeImm(inst)) + hs.pc;
    return (op == isa::inst::Opcode::rv32i_jal && jmp_target == hs.pc);
}

void Hart::init() {
    // allocate a stack region at 0x7fffffff00000000-0x7fffffff00010000
    hs.rf.GPR[2] = 0x7fffffff00010000;
    hs.memimg.allocate(0x7fffffff00000000, 0x10008);
}

void Hart::execute(uint64_t start_address) {
    hs.pc = start_address;
    while(1) {
        try {
            auto inst = hs.getInstWord();
            trace_inst << "PC[" << Trace::doubleword << hs.pc << "] = ";
            trace_inst << Trace::word << inst;
            trace_inst << "; " << isa::inst::disassemble(inst, hs.pc);
            trace_inst << std::endl;

            if(shouldHalt()) break;
            isa::inst::executeInstruction(inst, hs);
        } catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
            break;
        }
    }
}

} // namespace cpu
