#include "hartstate.h"

namespace hart {
// use raw(addr) so we don't log mem access
types::InstructionWord HartState::getInstWord() const {
    auto ptr = mem().raw(pc);
    return *((types::InstructionWord*)ptr);
}

HartState::HartState(std::shared_ptr<mem::MemoryImage> m)
    : rf_(std::make_unique<isa::rf::RegisterFile>()), memories_(),
      execution_state(ExecutionState::STOPPED) {
    // insert memory image for address space 0
    this->memories_.insert_or_assign(0, m);
}

} // namespace hart
