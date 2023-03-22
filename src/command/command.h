
#ifndef ZIRCON_COMMAND_COMMAND_H_
#define ZIRCON_COMMAND_COMMAND_H_

#include "expr.h"

#include "event/event.h"
#include "hart/hart.h"
#include "hart/isa/register.h"
#include "hart/types.h"
#include "mem/memory-image.h"

#include <deque>
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace action {
class ActionBase;
using ActionPtr = std::shared_ptr<ActionBase>;
} // namespace action

namespace command {

enum class CommandContext { CLI, REPL };

class Command;
using CommandPtr = std::shared_ptr<Command>;

class Command {
  protected:
    hart::HartState* hs;
    CommandContext context;
    std::vector<::action::ActionPtr> actions;
    command::ExprPtr condition;
    bool useColor;

  public:
    Command(
        hart::HartState* hs,
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        command::ExprPtr condition,
        bool useColor = false);

    Command(
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        command::ExprPtr condition,
        bool useColor = false)
        : Command(nullptr, context, actions, condition, useColor) {}
    Command(
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        bool useColor = false)
        : Command(nullptr, context, actions, nullptr, useColor) {}

    virtual ~Command() = default;
    virtual void setColor(bool useColor);
    virtual void setHS(hart::HartState* hs);
    virtual bool shouldDoit();
    virtual void doit([[maybe_unused]] std::ostream* o = nullptr);
    virtual void install() {
        // does nothing in the base case
    }
};

class CallbackCommand : public Command {
  protected:
    std::set<event::EventType> events;

  public:
    CallbackCommand(
        hart::HartState* hs,
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        command::ExprPtr condition,
        std::set<event::EventType> events,
        bool useColor = false);
    CallbackCommand(
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        command::ExprPtr condition,
        std::set<event::EventType> events,
        bool useColor = false)
        : CallbackCommand(
              nullptr,
              context,
              actions,
              condition,
              events,
              useColor) {}
    CallbackCommand(
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        std::set<event::EventType> events,
        bool useColor = false)
        : CallbackCommand(
              nullptr,
              context,
              actions,
              nullptr,
              events,
              useColor) {}

    CallbackCommand(
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        command::ExprPtr condition,
        bool useColor = false)
        : CallbackCommand(nullptr, context, actions, condition, {}, useColor) {}

    CallbackCommand(
        CommandContext context,
        std::vector<::action::ActionPtr> actions,
        bool useColor = false)
        : CallbackCommand(nullptr, context, actions, nullptr, {}, useColor) {}

    // override this classes doit, cannot call directly and should do nothing
    virtual void doit([[maybe_unused]] std::ostream* o = nullptr) override {}
    virtual void install() override;
};

} // namespace command

#endif
