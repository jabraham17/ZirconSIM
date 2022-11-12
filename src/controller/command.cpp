#include "command.h"
#include "common/format.h"

namespace controller {

namespace action {
void DumpRegisterClass::action(std::ostream* o) {
    if(o) {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            this->hs->rf.GPR.dump(*o);
            *o << std::endl;
        }

    }
}
template <size_t NUM, size_t SIZE>
void DumpRegister<NUM, SIZE>::action(std::ostream* o) {
    if(o) this->rc->rawreg.dump(*o);
}

void DumpPC::action(std::ostream* o) {
    if(o) *o << common::Format::doubleword << hs->pc << "\n";
}
void DumpMemoryAddress::action(std::ostream* o) {
    if(o)
        *o << "MEM[" << common::Format::doubleword << mi->raw(addr)
           << "] = " << common::Format::doubleword
           << *(uint64_t*)(mi->raw(addr)) << "\n";
}
void Stop::action([[maybe_unused]] std::ostream* o) { hs->executing = false; }
} // namespace action
} // namespace controller
