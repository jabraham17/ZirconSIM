#include "command.h"
#include "common/format.h"
#include "cpu/isa/inst.h"

namespace controller {

namespace action {
void DumpRegisterClass::action(std::ostream* o) {
    if(o && hs) {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            this->hs->rf.GPR.dump(*o);
            *o << std::endl;
        }
    }
}
void DumpRegister::action(std::ostream* o) {
    if(o && hs) {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            this->hs->rf.GPR.rawreg(idx).dump(*o);
            *o << std::endl;
        }
    }
}

void DumpPC::action(std::ostream* o) {
    if(o && hs) {
        uint64_t pc = hs->pc;
        uint32_t inst = 0;
        if(hs->memimg.raw(pc)) inst = hs->getInstWord();
        *o << "PC[" << common::Format::doubleword << pc
           << "] = " << isa::inst::disassemble(inst, pc) << std::endl;
    }
}
void DumpMemoryAddress::action(std::ostream* o) {
    if(o && hs) {
        auto converted_addr = hs->memimg.raw(addr);
        *o << "MEM[" << common::Format::doubleword << addr
           << "] = " << common::Format::doubleword
           << *(uint64_t*)(converted_addr) << std::endl;
    }
}
void Stop::action([[maybe_unused]] std::ostream* o) {
    if(hs) hs->executing = false;
}
} // namespace action
} // namespace controller
