#include "hartstate.h"

namespace hart {
// use raw(addr) so we don't log mem access
types::InstructionWord HartState::getInstWord() const {
    auto ptr = mem().raw(pc);
    return *((types::InstructionWord*)ptr);
}

HartState::HartState(std::shared_ptr<mem::MemoryImage> m)
    : rf_(std::make_unique<isa::rf::RegisterFile>()), memimg_(m),
      execution_state(ExecutionState::STOPPED) {}

} // namespace hart
