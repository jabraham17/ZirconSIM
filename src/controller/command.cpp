#include "command.h"
#include "color/color.h"
#include "common/format.h"
#include "cpu/isa/inst.h"

namespace controller {

static std::string colorAddr(bool useColor) {
    return useColor
               ? color::getColor(
                     {color::ColorCode::LIGHT_CYAN, color::ColorCode::FAINT})
               : "";
}

static std::string colorHex(bool useColor) {
    return useColor ? color::getColor(
                          {color::ColorCode::CYAN, color::ColorCode::FAINT})
                    : "";
}
static std::string colorNew(bool useColor) {
    return useColor ? color::getColor(
                          {color::ColorCode::GREEN, color::ColorCode::FAINT})
                    : "";
}
static std::string colorOld(bool useColor) {
    return useColor ? color::getColor(
                          {color::ColorCode::RED, color::ColorCode::FAINT})
                    : "";
}
static std::string colorReset(bool useColor) {
    return useColor ? color::getReset() : "";
}

namespace action {
// TODO: ADD COLOR
void DumpRegisterClass::action(std::ostream* o) {
    if(o && hs) {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            *o << std::string(indent, ' ');
            this->hs->rf.GPR.dump(*o);
            *o << std::endl;
        }
    }
}
// TODO: add color
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
        uint64_t pc = hs->pc + offset * 4;
        uint32_t inst = 0;
        uint32_t* inst_ptr = reinterpret_cast<uint32_t*>(hs->memimg.raw(pc));
        if(inst_ptr) inst = *inst_ptr;
        *o << std::string(indent, ' ');
        *o << "PC[" << colorAddr(useColor) << common::Format::doubleword << pc
           << colorReset(useColor) << "] = " << colorHex(useColor)
           << common::Format::word << pc << colorReset(useColor) << "; "
           << isa::inst::disassemble(inst, pc, useColor) << std::endl;
    }
}
void DumpMemoryAddress::action(std::ostream* o) {
    if(o && hs) {
        *o << std::string(indent, ' ');
        *o << "MEM[" << colorAddr(useColor) << common::Format::doubleword
           << addr << colorReset(useColor) << "] = ";
        auto converted_addr = hs->memimg.raw(addr);
        if(converted_addr) {
            auto value = *(uint64_t*)(converted_addr);
            *o << colorNew(useColor) << common::Format::doubleword << value
               << colorReset(useColor);
            if(isa::inst::decodeInstruction(*(uint32_t*)(converted_addr)) !=
               isa::inst::Opcode::UNKNOWN) {
                *o << " disassembles to "
                   << isa::inst::disassemble(
                          *(uint32_t*)(converted_addr),
                          0,
                          useColor);
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
    auto colorWatch = [](bool useColor) {
        return useColor ? color::getColor({color::ColorCode::RED}) : "";
    };
    // no value, read one
    if(!previous.has_value()) {
        if(out)
            *out << colorWatch(useColor) << "WATCH " << name()
                 << colorReset(useColor) << ": Setting initial value to "
                 << colorNew(useColor) << common::Format::doubleword << *current
                 << colorReset(useColor) << std::endl;
        previous = *current;
        updated = true;
    }
    // update if not the same
    else if(*previous != current) {
        if(out) {
            *out << colorWatch(useColor) << "WATCH " << name()
                 << colorReset(useColor) << ": NEW=" << colorNew(useColor)
                 << common::Format::doubleword << *current
                 << colorReset(useColor) << "; PREV=" << colorOld(useColor)
                 << common::Format::doubleword << *previous
                 << colorReset(useColor) << std::endl;
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
