#include "action.h"

#include "color/color.h"
#include "common/debug.h"
#include "common/format.h"
#include "hart/isa/inst-execute.h"
#include "hart/isa/inst.h"

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
static std::string colorReset(bool useColor) {
    return useColor ? color::getReset() : "";
}

namespace action {
std::set<event::EventType>
getDefaultEventsForAction(ActionType at, ::command::CommandContext cc) {
    // empty set means it wil execute immediately
    auto allEvents = event::getAllEventTypes();
    if(cc == command::CommandContext::CLI) {
        switch(at) {
            case ActionType::STOP:
                return {event::EventType::HART_BEFORE_EXECUTE};
            case ActionType::PAUSE:
                return {event::EventType::HART_BEFORE_EXECUTE};
            case ActionType::RESUME:
                return {event::EventType::HART_BEFORE_EXECUTE};
            case ActionType::DISASM:
                return {event::EventType::HART_BEFORE_EXECUTE};
            case ActionType::DUMP:
                return {event::EventType::HART_AFTER_EXECUTE};
            case ActionType::WATCH:
                return std::set(allEvents.begin(), allEvents.end());
            case ActionType::SET:
                return {event::EventType::HART_BEFORE_EXECUTE};
            default: return {};
        }
    } else if(cc == command::CommandContext::REPL) {
        switch(at) {
            case ActionType::WATCH:
                return std::set(allEvents.begin(), allEvents.end());
            default: return {};
        }
    } else {
        return {};
    }
}

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
void Set::action([[maybe_unused]] std::ostream* o) {
    if(hs && expr1 && expr2) {
        expr1->set(hs, expr2);
    }
}
void Dump::action(std::ostream* o) {
    if(o && hs) {
        *o << std::string(indent, ' ');
        std::string sep;
        for(auto a : args) {
            *o << " ";
            if(auto expr = std::get_if<command::ExprPtr>(&a)) {
                auto val = (*expr)->eval(hs);
                *o << common::Format::dec;
                *o << val;
            } else if(auto str = std::get_if<std::string>(&a)) {
                *o << *str;
            } else {
                std::cerr << "UNKNOWN ARGUMENT FOR DUMP: PANIC!!!\n";
                exit(1);
            }
            sep = " ";
        }
        *o << std::endl;
    }
}
void Watch::action([[maybe_unused]] std::ostream* o) {
    if(!hs) return;
    auto current = static_cast<types::UnsignedInteger>(this->expr->eval(hs));

    // either this is the first value set or a change
    bool valueIsDifferent = !previous.has_value() || *previous != current;
    if(valueIsDifferent) {
        previous = current;

        for(auto a : actions) {
            a->action(o);
        }
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
