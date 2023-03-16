#include "command.h"

#include "action.h"

#include "common/debug.h"
#include "hart/isa/inst-execute.h"
#include "hart/isa/inst.h"

namespace command {

Command::Command(
    hart::HartState* hs,
    CommandContext context,
    std::vector<::action::ActionPtr> actions,
    command::ExprPtr condition,
    bool useColor)
    : hs(hs), context(context), actions(actions), condition(condition),
      useColor(useColor) {
    assert(
        actions.size() > 0 && "a command needs at least one command to "
                              "execute");
    for(auto a : this->actions) {
        a->setHS(this->hs);
        a->setColor(this->useColor);
    }
}
void Command::setColor(bool useColor) {
    this->useColor = useColor;
    for(auto a : this->actions) {
        a->setColor(this->useColor);
    }
}
void Command::setHS(hart::HartState* hs) {
    this->hs = hs;
    for(auto a : this->actions) {
        a->setHS(this->hs);
    }
}
bool Command::shouldDoit() {
    // do it if hs and either there is no condition or condition evals to
    // true
    return hs && (!condition || bool(condition->eval(hs)));
}
void Command::doit(std::ostream* o) {
    if(shouldDoit()) {
        for(auto a : actions) {
            a->action(o);
        }
    }
}

CallbackCommand::CallbackCommand(
    hart::HartState* hs,
    CommandContext context,
    std::vector<::action::ActionPtr> actions,
    command::ExprPtr condition,
    std::set<event::EventType> events,
    bool useColor)
    : Command(hs, context, actions, condition, useColor), events(events) {
    if(this->events.empty()) {
        // no events, use the union of the defaults for the actions
        std::deque<::action::ActionPtr> q(
            this->actions.begin(),
            this->actions.end());
        while(!q.empty()) {
            auto a = q.back();
            q.pop_back();

            // add default events
            auto new_events = ::action::getDefaultEventsForAction(
                a->getType(),
                this->context);
            this->events.insert(new_events.begin(), new_events.end());

            // add nested actions
            if(a->isa<::action::ActionGroup>()) {
                auto more_actions =
                    a->cast<::action::ActionGroup>()->getActions();
                q.insert(q.begin(), more_actions.begin(), more_actions.end());
            }
        }
    }
}

void CallbackCommand::install(hart::Hart* hart) {
    if(!hs || !hart) return;
// attach to all relevant events
// use super classes doit
#define CALLBACK_TO_INSTALL this->Command::doit(&std::cout);

    for(auto event_type : this->events) {
        switch(event_type) {
            case event::EventType::HART_BEFORE_EXECUTE:
                hart->addBeforeExecuteListener(
                    [this](hart::HartState&) { CALLBACK_TO_INSTALL });
                break;
            case event::EventType::HART_AFTER_EXECUTE:
                hart->addAfterExecuteListener(
                    [this](hart::HartState&) { CALLBACK_TO_INSTALL });
                break;
            case event::EventType::MEM_READ:
                hart->hs().mem().addReadListener(
                    [this](uint64_t, uint64_t, size_t) { CALLBACK_TO_INSTALL });
                break;
            case event::EventType::MEM_WRITE:
                hart->hs().mem().addWriteListener(
                    [this](uint64_t, uint64_t, uint64_t, size_t) {
                        CALLBACK_TO_INSTALL
                    });
                break;
            case event::EventType::MEM_ALLOCATION:
                hart->hs().mem().addAllocationListener(
                    [this](uint64_t, uint64_t) { CALLBACK_TO_INSTALL });
                break;
            case event::EventType::REG_READ:
                hart->hs().rf().addReadListener(
                    [this](std::string, uint64_t, uint64_t) {
                        CALLBACK_TO_INSTALL
                    });
                break;
            case event::EventType::REG_WRITE:
                hart->hs().rf().addWriteListener(
                    [this](std::string, uint64_t, uint64_t, uint64_t) {
                        CALLBACK_TO_INSTALL
                    });
            default: std::cerr << "No Event Handler Defined\n";
        }
    }
#undef CALLBACK_TO_INSTALL
}

} // namespace command
