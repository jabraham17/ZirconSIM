#include "command.h"
#include "common/format.h"
#include "cpu/isa/inst.h"

namespace controller {

namespace action {
void DumpRegisterClass::action(std::ostream* o) {
    if(o && hs) {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            *o << std::string(indent, ' ');
            this->hs->rf.GPR.dump(*o);
            *o << std::endl;
        }
    }
}
void DumpRegister::action(std::ostream* o) {
    if(o && hs) {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            *o << std::string(indent, ' ');
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
        *o << std::string(indent, ' ');
        *o << "PC[" << common::Format::doubleword << pc
           << "] = " << isa::inst::disassemble(inst, pc) << std::endl;
    }
}
void DumpMemoryAddress::action(std::ostream* o) {
    if(o && hs) {
        *o << std::string(indent, ' ');
        *o << "MEM[" << common::Format::doubleword << addr << "] = ";
        auto converted_addr = hs->memimg.raw(addr);
        if(converted_addr) {
            auto value = *(uint64_t*)(converted_addr);
            *o << common::Format::doubleword << value;
            if(isa::inst::decodeInstruction(*(uint32_t*)(converted_addr)) !=
               isa::inst::Opcode::UNKNOWN) {
                *o << " disassembles to "
                   << isa::inst::disassemble(*(uint32_t*)(converted_addr));
            }
        } else *o << "nil";
    }

    *o << std::endl;
}
void Stop::action([[maybe_unused]] std::ostream* o) {
    if(hs) hs->executing = false;
}
void ActionGroup::action([[maybe_unused]] std::ostream* o) {
    if(hs && !actions.empty()) {
        *o << std::string(indent, ' ');
        *o << "ACTION GROUP" << std::endl;
        for(auto a : actions) {
            a->action(o);
        }
    }
}
} // namespace action

void Watch::update() {
    std::optional<uint64_t> current = readCurrentValue();
    if(!current.has_value()) return;
    bool updated = false;
    // no value, read one
    if(!previous.has_value()) {
        if(out)
            *out << "WATCH " << name() << ": Setting initial value to "
                 << common::Format::doubleword << *current << std::endl;
        previous = *current;
        updated = true;
    }
    // update if not the same
    else if(*previous != current) {
        if(out) {
            *out << "WATCH " << name()
                 << ": PREV=" << common::Format::doubleword << *previous
                 << " NEW=" << common::Format::doubleword << *current
                 << std::endl;
        }
        previous = *current;
        updated = true;
    }

    if(updated) {
        for(auto a : actions) {
            a->action(out);
        }
    }
}

} // namespace controller
