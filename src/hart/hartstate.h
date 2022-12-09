#ifndef ZIRCON_HART_HARTSTATE_H_
#define ZIRCON_HART_HARTSTATE_H_

#include "isa/rf.h"
#include "mem/memory-image.h"

#include <memory>
#include <unordered_map>

namespace hart {

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

} // namespace hart

#endif
