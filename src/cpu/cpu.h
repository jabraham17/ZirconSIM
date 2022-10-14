
#ifndef ZIRCON_CPU_CPU_H_
#define ZIRCON_CPU_CPU_H_

#include "isa/rf.h"
#include "mem/memory-image.h"
#include "trace/trace.h"
#include "trace/stats.h"

namespace cpu {

class HartState {
  public:
    isa::rf::RegisterFile rf;
    mem::MemoryImage& memimg;
    // address in memory of current instruction
    uint64_t pc;
    // use raw(addr) so we don't log mem access
    uint32_t getInstWord() const;

    HartState(mem::MemoryImage& m, TraceMode tm = TraceMode::NONE);
};

struct CPUException : public std::exception {
    const char* what() const throw() { return "CPU Exception"; }
};
struct IllegalInstructionException : public CPUException {
    const char* what() const throw() { return "Illegal Instruction"; }
};

class Hart {
  private:
    HartState hs;
    TraceMode trace_mode;
    Trace trace_inst;
    Stats stats;
    bool shouldHalt();

  public:
    Hart(mem::MemoryImage& m, TraceMode tm = TraceMode::NONE, bool useStats = false);
    void init();
    void execute(uint64_t start_address);
};

} // namespace cpu

#endif
