
#ifndef ZIRCON_CPU_CPU_H_
#define ZIRCON_CPU_CPU_H_

#include "event/event.h"
#include "isa/rf.h"
#include "mem/memory-image.h"
#include <map>

namespace cpu {

class HartState {
  public:
    isa::rf::RegisterFile rf;
    mem::MemoryImage& memimg;

    std::map<std::string, uint64_t> memory_locations;

    struct PCProxy {
        using T = uint64_t;

      private:
        T current_pc;
        T previous_pc;

      public:
        T read() { return current_pc; }
        T read() const { return current_pc; }
        void write(T v) {
            previous_pc = current_pc;
            current_pc = v;
        }
        T previous() { return previous_pc; }
        operator T() { return read(); }
        operator T() const { return read(); }
        PCProxy operator=(T v) {
            write(v);
            return *this;
        }
        PCProxy operator+=(T v) {
            write(current_pc + v);
            return *this;
        }
        friend std::ostream& operator<<(std::ostream& os, PCProxy pc) {
            os << pc.current_pc;
            return os;
        }
    };

    // address in memory of current instruction
    PCProxy pc;
    // use raw(addr) so we don't log mem access
    uint32_t getInstWord() const;

    bool executing;

    HartState(mem::MemoryImage& m);
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
    bool shouldHalt();

  public:
    Hart(mem::MemoryImage& m);
    void init_heap();
    void init_stack();
    void init();
    void execute(uint64_t start_address);

    event::Event<HartState&> event_before_execute;
    event::Event<HartState&> event_after_execute;
    template <typename T> void addRegisterReadListener(T&& arg) {
        hs.rf.addReadListener(std::forward<T>(arg));
    }
    template <typename T> void addRegisterWriteListener(T&& arg) {
        hs.rf.addWriteListener(std::forward<T>(arg));
    }
};

} // namespace cpu

#endif
