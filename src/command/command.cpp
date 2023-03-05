#include "command.h"

#include "color/color.h"
#include "common/debug.h"
#include "common/format.h"
#include "hart/isa/inst-execute.h"
#include "hart/isa/inst.h"

namespace command {

Command::Command(
    std::vector<std::shared_ptr<action::ActionInterface>> actions,
    command::ExprPtr condition,
    std::vector<event::EventType> events)
    : actions(actions), condition(condition), events(events), hs(nullptr) {
    // if not defined on any events, should be defined on the default event
    if(this->events.empty()) {
        this->events = event::getDefaultEventTypes();
    }
}

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
void Stop::action([[maybe_unused]] std::ostream* o) {
    if(hs) hs->stop();
}
void Pause::action([[maybe_unused]] std::ostream* o) {
    if(hs) hs->pause();
}
void Resume::action([[maybe_unused]] std::ostream* o) {
    if(hs) hs->resume();
}
void Disasm::action(std::ostream* o) {
    if(o && hs && expr) {
        auto pc = expr->eval(hs);
        uint32_t inst = 0;
        if(pc) {
            uint32_t* inst_ptr = reinterpret_cast<uint32_t*>(hs->mem().raw(pc));
            if(inst_ptr) inst = *inst_ptr;
        }
        *o << std::string(indent, ' ');
        *o << "PC[" << colorAddr(useColor) << common::Format::doubleword << pc
           << colorReset(useColor) << "] = " << colorHex(useColor)
           << common::Format::word << pc << colorReset(useColor) << "; "
           << isa::inst::disassemble(inst, pc, useColor) << std::endl;
    }
}
void Dump::action(std::ostream* o) {
    if(o && hs && expr) {
        auto val = expr->eval(hs);
        *o << std::string(indent, ' ');
        *o << expr->getString() << " = ";
        *o << common::Format::dec;
        *o << val;
        *o << std::endl;
    }
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
    std::optional<types::UnsignedInteger> current = readCurrentValue();
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

std::string WatchMemoryAddress::name() {
    std::stringstream ss;

    ss << "MEM[";
    if(hs && address) ss << common::Format::doubleword << address->eval(hs);
    else ss << "<unknown>]";
    return ss.str();
}
std::optional<types::UnsignedInteger> WatchMemoryAddress::readCurrentValue() {
    if(hs && address) {
        auto eval_addr = address->eval(hs);
        auto converted_addr = hs->mem().raw(eval_addr);
        if(converted_addr) return *(types::Address*)(converted_addr);
    }
    return std::nullopt;
}

} // namespace command
