#ifndef ZIRCON_HART_HARTSTATE_H_
#define ZIRCON_HART_HARTSTATE_H_

#include "common/debug.h"
#include "isa/rf.h"
#include "mem/memory-image.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace command {
struct Expr;
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
    std::shared_ptr<mem::MemoryImage> memimg_;

    std::unordered_map<std::string, types::Address> memory_locations_;

    std::mutex lock_es;
    std::condition_variable signal_es;
    ExecutionState execution_state;

  public:
    isa::rf::RegisterFile& rf() { return *rf_; }
    mem::MemoryImage& mem() { return *memimg_; }
    const isa::rf::RegisterFile& rf() const { return *rf_; }
    const mem::MemoryImage& mem() const { return *memimg_; }

    types::Address getMemLocation(const std::string& name) {
        if(auto it = memory_locations_.find(name);
           it != memory_locations_.end())
            return it->second;
        else return 0;
    }
    void setMemLocation(const std::string& name, types::UnsignedInteger val) {
        memory_locations_.insert_or_assign(name, val);
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

    void start(types::Address start_address) {
        pc = start_address;
        setExecutionState(ExecutionState::RUNNING);
    }
    void stop() { setExecutionState(ExecutionState::STOPPED); }
    void pause() { setExecutionState(ExecutionState::PAUSED); }
    void resume() { setExecutionState(ExecutionState::RUNNING); }
    ExecutionState getExecutionState() { return execution_state; }
    bool isRunning() { return execution_state == ExecutionState::RUNNING; }
    bool isPaused() { return execution_state == ExecutionState::PAUSED; }

    void setExecutionState(ExecutionState es) {
        if(!isValidExecutionStateTransition(es)) {
            es = ExecutionState::INVALID_STATE;
            common::debug::logln("Hart is now in an invalid execution state");
        }
        {
            std::unique_lock lk(lock_es);
            execution_state = es;
        }

        signal_es.notify_all();
    }
    template <typename Callable, typename... Args>
    void waitForExecutionStateChange(Callable func, Args... args) {
        {
            std::unique_lock lk(lock_es);
            signal_es.wait(lk);
            func(args...);
        }
    }
    void waitForExecutionStateChange() {
        std::unique_lock lk(lock_es);
        signal_es.wait(lk);
    }

  private:
    bool isValidExecutionStateTransition(ExecutionState new_es) {
        if(new_es == ExecutionState::INVALID_STATE) return true;
        switch(execution_state) {
            case ExecutionState::INVALID_STATE: return false;
            case ExecutionState::PAUSED:
                return new_es == ExecutionState::RUNNING ||
                       new_es == ExecutionState::STOPPED;
            case ExecutionState::RUNNING:
                return new_es == ExecutionState::STOPPED ||
                       new_es == ExecutionState::PAUSED;
            case ExecutionState::STOPPED:
                return new_es == ExecutionState::RUNNING;
        }
        return false;
    }
};

} // namespace hart

#endif
