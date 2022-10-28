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

Hart::Hart(mem::MemoryImage& m, TraceMode tm, bool useStats, bool doColor)
    : hs(m, tm), trace_mode(tm), trace_inst("INSTRUCTION TRACE"),
      doColor(doColor) {
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

char* mystr = "hello world";

uint64_t write_str(HartState& hs, char* s) {
    int len = strlen(s) + 1; // need to inlcude nullbyte
    auto ptr = hs.memory_locations["heap_start"];
    hs.memimg.allocate(hs.memory_locations["heap_start"], len);
    hs.memory_locations["heap_start"] += len;
    for(int i = 0; i < len; i++) {
        hs.memimg.byte(ptr + i) = s[i];
    }
    return ptr;
}

enum class AUXVecType: uint64_t {
    AT_NULL = 0,    /* End of vector */
    AT_IGNORE = 1,  /* Entry should be ignored */
    AT_EXECFD = 2,  /* File descriptor of program */
    AT_PHDR = 3,    /* Program headers for program */
    AT_PHENT = 4,   /* Size of program header entry */
    AT_PHNUM = 5,   /* Number of program headers */
    AT_PAGESZ = 6,  /* System page size */
    AT_BASE = 7,    /* Base address of interpreter */
    AT_FLAGS = 8,   /* Flags */
    AT_ENTRY = 9,   /* Entry point of program */
    AT_NOTELF = 10, /* Program is not ELF */
    AT_UID = 11,    /* Real uid */
    AT_EUID = 12,   /* Effective uid */
    AT_GID = 13,    /* Real gid */
    AT_EGID = 14,   /* Effective gid */
    AT_CLKTCK = 17, /* Frequency of times() */

    /* Some more special a_type values describing the hardware.  */
    AT_PLATFORM = 15, /* String identifying platform.  */
    AT_HWCAP = 16,    /* Machine-dependent hints about
                                 processor capabilities.  */

    /* This entry gives some information about the FPU initialization
       performed by the kernel.  */
    AT_FPUCW = 18, /* Used FPU control word.  */

    /* Cache block sizes.  */
    AT_DCACHEBSIZE = 19, /* Data cache block size.  */
    AT_ICACHEBSIZE = 20, /* Instruction cache block size.  */
    AT_UCACHEBSIZE = 21, /* Unified cache block size.  */

    /* A special ignored value for PPC, used by the kernel to control the
       interpretation of the AUXV. Must be > 16.  */
    AT_IGNOREPPC = 22, /* Entry should be ignored.  */

    AT_SECURE = 23, /* Boolean, was exec setuid-like?  */

    AT_BASE_PLATFORM = 24, /* String identifying real platforms.*/

    AT_RANDOM = 25, /* Address of 16 random bytes.  */

    AT_HWCAP2 = 26, /* More machine-dependent hints about
                               processor capabilities.  */

    AT_EXECFN = 31, /* Filename of executable.  */

    /* Pointer to the global system page used for system calls and other
       nice things.  */
    AT_SYSINFO = 32,
    AT_SYSINFO_EHDR = 33,

    /* Shapes of the caches.  Bits 0-3 contains associativity; bits 4-7 contains
       log2 of line size; mask those to get cache size.  */
    AT_L1I_CACHESHAPE = 34,
    AT_L1D_CACHESHAPE = 35,
    AT_L2_CACHESHAPE = 36,
    AT_L3_CACHESHAPE = 37,

    /* Shapes of the caches, with more room to describe them.
       *GEOMETRY are comprised of cache line size in bytes in the bottom 16 bits
       and the cache associativity in the next 16 bits.  */
    AT_L1I_CACHESIZE = 40,
    AT_L1I_CACHEGEOMETRY = 41,
    AT_L1D_CACHESIZE = 42,
    AT_L1D_CACHEGEOMETRY = 43,
    AT_L2_CACHESIZE = 44,
    AT_L2_CACHEGEOMETRY = 45,
    AT_L3_CACHESIZE = 46,
    AT_L3_CACHEGEOMETRY = 47,

    AT_MINSIGSTKSZ = 51, /* Stack needed for signal delivery  */
};

void Hart::init_stack() {
    // allocate a stack region at 0x7fffffff00000000-0x7fffffff00010000
    hs.memory_locations["start_start"] = 0x7fffffff00000000;
    uint64_t stack_size = 0x10000;
    hs.memory_locations["stack_end"] =
        hs.memory_locations["start_start"] + stack_size;
    hs.memimg.allocate(hs.memory_locations["start_start"], stack_size);

    auto sp = hs.memory_locations["stack_end"];

    // start stack pointer somewhere inside the stack, near the end, 128 bit
    // aligned
    sp = sp - 0x1000;

    // abpve sp goes all the stack crap, main starts at sp

    // put auxvec
    hs.memimg.doubleword(sp+64) = 0;
    hs.memimg.doubleword(sp+56) = uint64_t(AUXVecType::AT_NULL);
    hs.memimg.doubleword(sp + 48) = 4096;
    hs.memimg.doubleword(sp + 40) = uint64_t(AUXVecType::AT_PAGESZ);

    // env
    hs.memimg.doubleword(sp + 32) = 0;
    // rest of env

    // argv
    hs.memimg.doubleword(sp + 24) = 0;
    hs.memimg.doubleword(sp + 16) = write_str(hs, "testing");
    hs.memimg.doubleword(sp + 8) = write_str(hs, mystr);

    // rest of argv

    // argc
    hs.memimg.word(sp) = 2;

    hs.rf.GPR[2] = sp;
}

void Hart::init_heap() {
    // initial heap size is 0
    uint64_t SPACING = 0x10000;
    hs.memory_locations["heap_start"] = 0x100000000;

    // hs.memory_locations["stack_end"] + SPACING;
    uint64_t heap_size = 0;
    hs.memory_locations["heap_end"] =
        hs.memory_locations["heap_start"] + heap_size;
    hs.memimg.allocate(hs.memory_locations["heap_start"], heap_size);
}

void Hart::init() {

    init_heap();
    init_stack();
}

void Hart::execute(uint64_t start_address) {
    hs.pc = start_address;
    while(1) {
        try {
            auto inst = hs.getInstWord();
            trace_inst << "PC[" << Trace::doubleword << hs.pc << "] = ";
            trace_inst << Trace::word << inst;
            trace_inst << "; " << isa::inst::disassemble(inst, hs.pc, doColor);
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
