
#ifndef ZIRCON_CPU_CPU_H_
#define ZIRCON_CPU_CPU_H_

#include "common/ordered_map.h"
#include "event/event.h"
#include "isa/rf.h"
#include "mem/memory-image.h"

namespace cpu {

class HartState {
  private:
    std::unique_ptr<isa::rf::RegisterFile> rf_;
    std::shared_ptr<mem::MemoryImage> memimg_;

    std::unordered_map<std::string, uint64_t> memory_locations_;

  public:
    isa::rf::RegisterFile& rf() { return *rf_; }
    mem::MemoryImage& mem() { return *memimg_; }
    const isa::rf::RegisterFile& rf() const { return *rf_; }
    const mem::MemoryImage& mem() const { return *memimg_; }

    uint64_t getMemLocation(const std::string& name) {
        if(auto it = memory_locations_.find(name);
           it != memory_locations_.end())
            return it->second;
        else return 0;
    }
    void setMemLocation(const std::string& name, uint64_t val) {
        memory_locations_.insert_or_assign(name, val);
    }

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

    HartState(std::shared_ptr<mem::MemoryImage> m);

    HartState& operator()() { return *this; }
    const HartState& operator()() const { return *this; }
};

struct CPUException : public std::exception {
    const char* what() const throw() { return "CPU Exception"; }
};
struct IllegalInstructionException : public CPUException {
    const char* what() const throw() { return "Illegal Instruction"; }
};

class Hart {
  private:
    std::unique_ptr<HartState> hs_;

  private:
    uint64_t alloc(size_t n);
    void copyToHart(void* src, uint64_t dst, size_t n);

    void init_heap();
    void init_stack(
        std::vector<std::string> argv = {},
        common::ordered_map<std::string, std::string> envp = {});
    bool shouldHalt();
    // Subsystem: hart
    // Description: Fires just before current instruction is executed
    // Parameters: (Hart State object)
    event::Event<HartState&> event_before_execute;
    // Subsystem: hart
    // Description: Fires just after current instruction is executed
    // Parameters: (Hart State object)
    event::Event<HartState&> event_after_execute;

  public:
    Hart(std::shared_ptr<mem::MemoryImage> m);
    void init(
        std::vector<std::string> argv = {},
        common::ordered_map<std::string, std::string> envp = {});
    void execute(uint64_t start_address);

    template <typename T> void addBeforeExecuteListener(T&& arg) {
        event_before_execute.addListener(std::forward<T>(arg));
    }
    template <typename T> void addAfterExecuteListener(T&& arg) {
        event_after_execute.addListener(std::forward<T>(arg));
    }
    template <typename T> void addRegisterReadListener(T&& arg) {
        hs().rf().addReadListener(std::forward<T>(arg));
    }
    template <typename T> void addRegisterWriteListener(T&& arg) {
        hs().rf().addWriteListener(std::forward<T>(arg));
    }
    HartState& hs() { return *hs_; }
};

} // namespace cpu

#endif
