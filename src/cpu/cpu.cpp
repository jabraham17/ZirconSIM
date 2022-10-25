#include "cpu.h"
#include "isa/inst.h"
#include "isa/instruction_match.h"
#include "isa/rf.h"
#include "syscall/syscall.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace cpu {

// use raw(addr) so we don't log mem access
uint32_t HartState::getInstWord() const { return *((uint32_t*)memimg.raw(pc)); }

HartState::HartState(mem::MemoryImage& m, TraceMode tm)
    : memimg(m), executing(true) {
    rf.setTraceMode(tm);
}

Hart::Hart(mem::MemoryImage& m, TraceMode tm, bool useStats)
    : hs(m, tm), trace_mode(tm), trace_inst("INSTRUCTION TRACE") {
    trace_inst.setState((trace_mode & TraceMode::INSTRUCTION));
    stats.setState(useStats);
}

bool Hart::shouldHalt() {
    // halt if no longer executing
    if(!hs.executing) return true;
    // if the instruction just executed was a jmp to itself, halt
    uint32_t inst = hs.getInstWord();
    auto op = isa::inst::decodeInstruction(inst);
    auto jmp_target =
        instruction::signext64<20>(instruction::getJTypeImm(inst)) + hs.pc;
    return (
        op == isa::inst::Opcode::rv32i_jal && jmp_target == hs.pc.previous());
}

void Hart::init() {
    // allocate a stack region at 0x7fffffff00000000-0x7fffffff00010000
    hs.memory_locations["start_start"] = 0x7fffffff00000000;
    uint64_t stack_size = 0x10000;
    hs.memory_locations["stack_end"] =
        hs.memory_locations["start_start"] + stack_size;
    hs.memimg.allocate(hs.memory_locations["start_start"], stack_size);
    // start stack pointer somewhere inside the stack, near the end, 128 bit
    // aligned
    hs.rf.GPR[2] = hs.memory_locations["stack_end"] - 0x100;

    // initial heap size is 0
    uint64_t SPACING = 0x10000;
    hs.memory_locations["heap_start"] = 0x1000;
        
        //hs.memory_locations["stack_end"] + SPACING;
    uint64_t heap_size = 0;
    hs.memory_locations["heap_end"] =
        hs.memory_locations["heap_start"] + heap_size;
    hs.memimg.allocate(hs.memory_locations["heap_start"], heap_size);
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
            stats.count(hs);

            isa::inst::executeInstruction(inst, hs);
            if(shouldHalt()) break;
        } catch(const std::exception& e) {
            std::cerr << "Exception Occurred: " << e.what() << std::endl;
            break;
        }
    }
    if(stats.isEnabled()) std::cout << stats.dump() << std::endl;
}

} // namespace cpu
