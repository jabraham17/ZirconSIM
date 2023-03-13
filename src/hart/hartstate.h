#ifndef ZIRCON_HART_HARTSTATE_H_
#define ZIRCON_HART_HARTSTATE_H_

#include "common/debug.h"
#include "isa/rf.h"
#include "mem/memory-image.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace command {
class Expr;
}

namespace hart {

enum class ExecutionState {
    INVALID_STATE,
    STOPPED,
    RUNNING,
    PAUSED,
};

class Hart;

class HartState {
    friend Hart;
    friend command::Expr;

  private:
    std::unique_ptr<isa::rf::RegisterFile> rf_;

    struct MemoryPair {
        std::shared_ptr<mem::MemoryImage> mem;
        std::unordered_map<std::string, types::Address> locations;
        MemoryPair(
            std::shared_ptr<mem::MemoryImage> mem,
            std::unordered_map<std::string, types::Address> locations = {})
            : mem(mem), locations(locations) {}
    };

    // address space 0 points to local_mem;
    std::unordered_map<types::Address, MemoryPair> memories_;

    // std::mutex lock_es;
    // std::condition_variable signal_es;
    std::atomic<ExecutionState> execution_state;

  public:
    isa::rf::RegisterFile& rf() const { return *rf_; }
    mem::MemoryImage& mem(types::Address addressSpace = 0) const {
        // TODO: need to handle case where address space not in memories
        if(auto mem_it = memories_.find(addressSpace);
           mem_it != memories_.end()) {
            return *(mem_it->second.mem);
        } else {
            std::cerr << "UNKNOWN ADDRESS SPACE: PANIC!!!\n";
            exit(1);
        }
    }
    isa::rf::RegisterFile& rf() {
        return const_cast<const HartState*>(this)->rf();
    }
    mem::MemoryImage& mem(types::Address addressSpace = 0) {
        return const_cast<const HartState*>(this)->mem(addressSpace);
    }

    types::Address
    getMemLocation(const std::string& name, types::Address addressSpace = 0) {
        if(auto mem_it = memories_.find(addressSpace);
           mem_it != memories_.end()) {

            auto locations = mem_it->second.locations;
            if(auto it = locations.find(name); it != locations.end())
                return it->second;
            else return 0;
        } else return 0;
    }
    void setMemLocation(
        const std::string& name,
        types::UnsignedInteger val,
        types::Address addressSpace = 0) {
        if(auto mem_it = memories_.find(addressSpace);
           mem_it != memories_.end()) {
            mem_it->second.locations.insert_or_assign(name, val);
        }
    }

    types::Address allocateAddressSpace() {
        static types::Address nextAddressSpace = 1;

        auto addrSpace = nextAddressSpace;
        nextAddressSpace++;
        memories_.insert_or_assign(
            addrSpace,
            std::make_shared<mem::MemoryImage>());
        return addrSpace;
    }

    struct PCProxy {
        using T = types::Address;

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
    types::InstructionWord getInstWord() const;

    HartState(std::shared_ptr<mem::MemoryImage> m);

    HartState& operator()() { return *this; }
    const HartState& operator()() const { return *this; }

    void setPC(types::Address start_address) { pc = start_address; }
    void start() { setExecutionState(ExecutionState::RUNNING); }
    void start(types::Address start_address) {
        setPC(start_address);
        start();
    }
    void stop() { setExecutionState(ExecutionState::STOPPED); }
    void pause() { setExecutionState(ExecutionState::PAUSED); }
    void resume() { setExecutionState(ExecutionState::RUNNING); }
    ExecutionState getExecutionState() { return execution_state; }
    bool isRunning() { return execution_state == ExecutionState::RUNNING; }
    bool isPaused() { return execution_state == ExecutionState::PAUSED; }
    bool isStopped() { return execution_state == ExecutionState::STOPPED; }
    bool isInInvalidState() {
        return execution_state == ExecutionState::INVALID_STATE;
    }

    void setExecutionState(ExecutionState es) {
        if(!isValidExecutionStateTransition(es)) {
            es = ExecutionState::INVALID_STATE;
            common::debug::logln("Hart is now in an invalid execution state");
        }
        execution_state = es;
    }

  private:
    bool isValidExecutionStateTransition(ExecutionState new_es) {
        if(new_es == ExecutionState::INVALID_STATE) return true;
        if(new_es == execution_state) return true;
        switch(execution_state.load()) {
            case ExecutionState::INVALID_STATE: return false;
            case ExecutionState::PAUSED:
                return new_es == ExecutionState::RUNNING ||
                       new_es == ExecutionState::STOPPED;
            case ExecutionState::RUNNING:
                return new_es == ExecutionState::STOPPED ||
                       new_es == ExecutionState::PAUSED;
            case ExecutionState::STOPPED:
                return new_es == ExecutionState::RUNNING ||
                       new_es == ExecutionState::PAUSED;
        }
        return false;
    }
};

} // namespace hart

#endif
