
#ifndef ZIRCON_COMMAND_COMMAND_H_
#define ZIRCON_COMMAND_COMMAND_H_

#include "expr.h"

#include "event/event.h"
#include "hart/hart.h"
#include "hart/isa/register.h"
#include "hart/types.h"
#include "mem/memory-image.h"

#include <memory>
#include <optional>

namespace command {

namespace action {
enum class ActionType {
    STOP,
    PAUSE,
    RESUME,
    DISASM,
    DUMP,
    GROUP,
    NONE,
};
class ActionInterface {
  public:
    ActionType at;

  protected:
    hart::HartState* hs;
    size_t indent;
    bool useColor = false;

  public:
    ActionInterface(
        ActionType at = ActionType::NONE,
        hart::HartState* hs = nullptr)
        : at(at), hs(hs), indent(0) {}
    virtual ~ActionInterface() = default;

    void operator()(std::ostream* o = nullptr) { this->action(o); }
    virtual void action(std::ostream* o = nullptr) = 0;

    template <typename U> bool isa() { return U::classof(this); }
    template <typename U> U* cast() {
        assert(this->isa<U>());
        return static_cast<U*>(this);
    }
    virtual void setHS(hart::HartState* hs) { this->hs = hs; }
    virtual void increaseIndent(size_t indent = 2) { this->indent += indent; }
    // FIXME: possible sign underflow may occur here
    virtual void decreaseIndent(size_t indent = 2) { this->indent -= indent; }
    virtual void setColor(bool useColor) { this->useColor = useColor; }
};

#define MAKE_ACTION_0_ARGS(ClassName, ActionTypeName)                          \
    class ClassName : public ActionInterface {                                 \
      public:                                                                  \
        ClassName() : ClassName(nullptr) {}                                    \
        ClassName(hart::HartState* hs)                                         \
            : ActionInterface(ActionType::ActionTypeName, hs) {}               \
        virtual ~ClassName() = default;                                        \
        void action(std::ostream* o = nullptr) override;                       \
        static bool classof(const ActionInterface* ai) {                       \
            return ai->at == ActionType::ActionTypeName;                       \
        }                                                                      \
    };
#define MAKE_ACTION_1_ARGS(ClassName, ActionTypeName)                          \
    class ClassName : public ActionInterface {                                 \
      private:                                                                 \
        command::ExprPtr expr;                                                 \
                                                                               \
      public:                                                                  \
        ClassName(command::ExprPtr expr) : ClassName(nullptr, expr) {}         \
        ClassName(hart::HartState* hs, command::ExprPtr expr)                  \
            : ActionInterface(ActionType::ActionTypeName, hs), expr(expr) {}   \
        virtual ~ClassName() = default;                                        \
        void action(std::ostream* o = nullptr) override;                       \
        static bool classof(const ActionInterface* ai) {                       \
            return ai->at == ActionType::ActionTypeName;                       \
        }                                                                      \
    };

MAKE_ACTION_0_ARGS(Stop, STOP)
MAKE_ACTION_0_ARGS(Pause, PAUSE)
MAKE_ACTION_0_ARGS(Resume, RESUME)
MAKE_ACTION_1_ARGS(Disasm, DISASM)
MAKE_ACTION_1_ARGS(Dump, DUMP)

class ActionGroup : public ActionInterface {
  private:
    std::vector<std::shared_ptr<action::ActionInterface>> actions;

  public:
    ActionGroup(std::vector<std::shared_ptr<action::ActionInterface>> actions)
        : ActionGroup(nullptr, actions) {}
    ActionGroup(
        hart::HartState* hs,
        std::vector<std::shared_ptr<action::ActionInterface>> actions)
        : ActionInterface(ActionType::GROUP, hs), actions(actions) {}
    virtual ~ActionGroup() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::GROUP;
    }
    virtual void setHS(hart::HartState* hs) override {
        ActionInterface::setHS(hs);
        updateActions();
    }
    virtual void setColor(bool useColor) override {
        ActionInterface::setColor(useColor);
        this->updateActions();
    }

    virtual void increaseIndent(size_t indent = 2) override {
        ActionInterface::increaseIndent(indent);
        for(auto a : this->actions) {
            a->increaseIndent();
        }
    }
    virtual void decreaseIndent(size_t indent = 2) override {
        ActionInterface::decreaseIndent(indent);
        for(auto a : this->actions) {
            a->decreaseIndent();
        }
    }

  private:
    void updateActions() {
        for(auto a : this->actions) {
            a->setHS(this->hs);
            a->increaseIndent();
            a->setColor(this->useColor);
        }
    }
};
} // namespace action

class Condition {
  protected:
    hart::HartState* hs;

  private:
    command::ExprPtr condition;

  public:
    Condition(hart::HartState* hs, command::ExprPtr condition)
        : hs(hs), condition(condition) {}
    Condition(command::ExprPtr condition) : Condition(nullptr, condition) {}
    Condition() : Condition(nullptr, nullptr) {}
    virtual ~Condition() = default;

    virtual bool check();
    virtual void setHS(hart::HartState* hs) { this->hs = hs; }
};

// empty class for inheritance
class ControlBase {
  protected:
    bool useColor = false;

  public:
    virtual ~ControlBase() = default;
    virtual void setColor(bool useColor) { this->useColor = useColor; }
    virtual void setHS(hart::HartState* hs) = 0;
};

class Command : public ControlBase {

  private:
    std::vector<std::shared_ptr<action::ActionInterface>> actions;
    std::vector<std::shared_ptr<Condition>> conditions;
    std::vector<event::EventType> events;

  public:
    Command(
        std::vector<std::shared_ptr<action::ActionInterface>> actions,
        std::vector<std::shared_ptr<Condition>> conditions,
        std::vector<event::EventType> events);
    virtual ~Command() = default;

    virtual void setHS(hart::HartState* hs) override {
        for(auto a : actions) {
            a->setHS(hs);
        }
        for(auto c : conditions) {
            c->setHS(hs);
        }
    }

    virtual bool shouldDoit() {
        // check if we should run the actions
        // if no conditions, shouldDoit is still true
        bool shouldDoit = true;
        for(auto c : conditions) {
            shouldDoit = shouldDoit && c->check();
        }
        return shouldDoit;
    }
    virtual void doit(std::ostream* o = nullptr) {
        if(shouldDoit()) {
            for(auto a : actions) {
                a->action(o);
            }
        }
    }
    auto getEventTypes() { return events; }

    virtual void setColor(bool useColor) override {
        ControlBase::setColor(useColor);
        for(auto a : actions)
            a->setColor(useColor);
    }
};

// change watches to define commands to also dump
// maybe restructure watches as a new operator?
// it becomes part of the conidition?

class Watch : public ControlBase {
  protected:
    std::optional<types::Address> previous;
    std::ostream* out;
    hart::HartState* hs;
    std::vector<std::shared_ptr<action::ActionInterface>> actions;

  public:
    Watch(
        hart::HartState* hs = nullptr,
        std::vector<std::shared_ptr<action::ActionInterface>> actions = {})
        : previous(), out(nullptr), hs(hs), actions(actions) {
        this->updateActions();
    }
    virtual ~Watch() = default;

    void setLog(std::ostream* o) { this->out = o; }
    virtual void setHS(hart::HartState* hs) override {
        this->hs = hs;
        this->updateActions();
    }
    void
    setActions(std::vector<std::shared_ptr<action::ActionInterface>> actions) {
        this->actions = actions;
        this->updateActions();
    }
    virtual void setColor(bool useColor) override {
        ControlBase::setColor(useColor);
        this->updateActions();
    }

    virtual bool hasChanged() {
        std::optional<types::Address> value = readCurrentValue();
        // no current value, no change
        if(!value.has_value()) return false;
        // no previous value, no change
        if(!previous.has_value()) return false;

        return *value != *previous;
    }
    virtual void update();
    virtual std::string name() = 0;
    virtual std::optional<types::UnsignedInteger> readCurrentValue() = 0;

  private:
    void updateActions() {
        for(auto a : this->actions) {
            a->setHS(this->hs);
            a->increaseIndent();
            a->setColor(this->useColor);
        }
    }
};

class WatchRegister : public Watch {
  public:
    isa::rf::RegisterSymbol reg;

    WatchRegister(isa::rf::RegisterSymbol reg)
        : WatchRegister(nullptr, {}, reg) {}
    WatchRegister(
        hart::HartState* hs,
        std::vector<std::shared_ptr<action::ActionInterface>> actions,
        isa::rf::RegisterSymbol reg)
        : Watch(hs, actions), reg(reg) {}
    virtual ~WatchRegister() = default;

    virtual std::string name() override {
        // TODO: probably want to provide a way to pass in the actual register
        // name used by the programmer, so we get better output
        return isa::rf::getRegisterClassString(this->reg.rct) + "[" +
               std::to_string(this->reg.idx) + "]";
    }
    virtual std::optional<types::UnsignedInteger> readCurrentValue() override {
        if(hs) {
            auto r = hs->rf().getRegisterClassForType(reg.rct);
            auto value = r.rawreg(reg.idx).get();
            return value;
        }
        return std::nullopt;
    }
};

class WatchMemoryAddress : public Watch {
  public:
    command::ExprPtr address;

    WatchMemoryAddress(command::ExprPtr address)
        : WatchMemoryAddress(nullptr, {}, address) {}
    WatchMemoryAddress(
        hart::HartState* hs,
        std::vector<std::shared_ptr<action::ActionInterface>> actions,
        command::ExprPtr address)
        : Watch(hs, actions), address(address) {}
    virtual ~WatchMemoryAddress() = default;

    virtual std::string name() override;
    virtual std::optional<types::UnsignedInteger> readCurrentValue() override;
};

} // namespace command

#endif
