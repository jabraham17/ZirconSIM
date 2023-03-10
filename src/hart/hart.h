
#ifndef ZIRCON_HART_HART_H_
#define ZIRCON_HART_HART_H_

#include "hartstate.h"
#include "types.h"

#include "common/ordered_map.h"
#include "event/event.h"
#include "isa/rf.h"
#include "mem/memory-image.h"

#include <thread>
#include "common/threading/syncpoint.h"

namespace hart {

struct HartException : public std::runtime_error {
    HartException() : std::runtime_error("Hart Exception") {}
    HartException(std::string message)
        : std::runtime_error("Hart Exception: " + message) {}
};
struct IllegalInstructionException : public HartException {
    IllegalInstructionException() : HartException("Illegal Instruction") {}
    IllegalInstructionException(types::InstructionWord bits)
        : HartException("Illegal Instruction [" + std::to_string(bits)) {}
};

class Hart {
  private:
    std::unique_ptr<HartState> hs_;

  private:
    types::Address alloc(size_t n);
    void copyToHart(void* src, types::Address dst, size_t n);

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

    void wait_till_done() {
        sync_point.wait();
        execution_thread.join();
    }
    void startExecution() {
        sync_point.signal();
    }
  private:
    common::threading::syncpoint sync_point;
    std::thread execution_thread;
    void execute();
};

} // namespace hart

#endif
