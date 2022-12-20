
#ifndef ZIRCON_COMMAND_COMMAND_H_
#define ZIRCON_COMMAND_COMMAND_H_

#include "event/event.h"
#include "hart/hart.h"
#include "hart/isa/register.h"
#include "mem/memory-image.h"
#include "hart/types.h"

#include <memory>
#include <optional>

namespace command {

namespace action {
enum class ActionType {
    DUMP_REG,
    DUMP_REG_CLASS,
    DUMP_MEM_ADDR,
    DUMP_PC,
    STOP,
    PAUSE,
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

class DumpRegisterClass : public ActionInterface {
  public:
    isa::rf::RegisterClassType regtype;

    DumpRegisterClass(isa::rf::RegisterClassType regtype)
        : DumpRegisterClass(nullptr, regtype) {}
    DumpRegisterClass(hart::HartState* hs, isa::rf::RegisterClassType regtype)
        : ActionInterface(ActionType::DUMP_REG_CLASS, hs), regtype(regtype) {}
    virtual ~DumpRegisterClass() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_REG_CLASS;
    }
};
class DumpRegister : public ActionInterface {
  public:
    isa::rf::RegisterClassType regtype;
    types::RegisterIndex idx;

    DumpRegister(isa::rf::RegisterClassType regtype, types::RegisterIndex idx)
        : DumpRegister(nullptr, regtype, idx) {}
    DumpRegister(
        hart::HartState* hs,
        isa::rf::RegisterClassType regtype,
        types::RegisterIndex idx)
        : ActionInterface(ActionType::DUMP_REG, hs), regtype(regtype),
          idx(idx) {}
    virtual ~DumpRegister() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_REG;
    }
};
class DumpPC : public ActionInterface {
  public:
    types::SignedInteger offset;
    DumpPC(types::SignedInteger offset) : DumpPC(nullptr, offset) {}
    DumpPC(hart::HartState* hs, types::SignedInteger offset)
        : ActionInterface(ActionType::DUMP_PC, hs), offset(offset) {}
    virtual ~DumpPC() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_PC;
    }
};
class DumpMemoryAddress : public ActionInterface {
  public:
    types::Address addr;

    DumpMemoryAddress(types::Address addr) : DumpMemoryAddress(nullptr, addr) {}
    DumpMemoryAddress(hart::HartState* hs, types::Address addr)
        : ActionInterface(ActionType::DUMP_MEM_ADDR, hs), addr(addr) {}
    virtual ~DumpMemoryAddress() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_MEM_ADDR;
    }
};
class Stop : public ActionInterface {
  public:
    Stop() : Stop(nullptr) {}
    Stop(hart::HartState* hs) : ActionInterface(ActionType::STOP, hs) {}
    virtual ~Stop() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::STOP;
    }
};
class Pause : public ActionInterface {
  public:
    Pause() : Pause(nullptr) {}
    Pause(hart::HartState* hs) : ActionInterface(ActionType::PAUSE, hs) {}
    virtual ~Pause() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::PAUSE;
    }
};
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

namespace condition {
enum class ConditionType {
    MEM_ADDR_CMP,
    REG_CMP,
    PC_CMP,
    ALWAYS_TRUE,
    NONE,
};
class ComparisonType {
  private:
    using ValueType = int;
    ValueType value_;

  public:
    static const ValueType NONE = 0;
    static const ValueType EQ = 1;
    static const ValueType NEQ = 2;
    static const ValueType LT = 3;
    static const ValueType GT = 4;
    static const ValueType LTE = 5;
    static const ValueType GTE = 6;

  public:
    ComparisonType(ValueType v) : value_(v) {}
    ComparisonType() : value_(NONE) {}
    ComparisonType& operator=(const ValueType& v) {
        this->value_ = v;
        return *this;
    }
    operator ValueType() const { return this->value_; }
    template <typename T> bool compare(T a, T b) {
        switch(this->value_) {
            case ComparisonType::EQ: return a == b;
            case ComparisonType::NEQ: return a != b;
            case ComparisonType::LT: return a < b;
            case ComparisonType::GT: return a > b;
            case ComparisonType::LTE: return a <= b;
            case ComparisonType::GTE: return a >= b;
            case ComparisonType::NONE:
            default: return false;
        }
    }
    template <typename T> bool operator()(T a, T b) { return compare<T>(a, b); }
};

class ConditionInterface {
  public:
    ConditionType ct;

  protected:
    hart::HartState* hs;

  public:
    ConditionInterface(
        ConditionType ct = ConditionType::NONE,
        hart::HartState* hs = nullptr)
        : ct(ct), hs(hs) {}
    virtual ~ConditionInterface() = default;

    virtual bool check() = 0;

    template <typename U> bool isa() { return U::classof(this); }
    template <typename U> U* cast() {
        assert(this->isa<U>());
        return static_cast<U*>(this);
    }
    virtual void setHS(hart::HartState* hs) { this->hs = hs; }
};
class MemAddrCompare : public ConditionInterface {
  public:
    types::Address addr;
    types::UnsignedInteger i;
    ComparisonType ct;

    MemAddrCompare(types::Address addr, types::UnsignedInteger i, ComparisonType ct)
        : MemAddrCompare(nullptr, addr, i, ct) {}
    MemAddrCompare(
        hart::HartState* hs,
        types::Address addr,
        types::UnsignedInteger i,
        ComparisonType ct)
        : ConditionInterface(ConditionType::MEM_ADDR_CMP, hs), addr(addr), i(i),
          ct(ct) {}
    virtual ~MemAddrCompare() = default;

    bool check() override {
        return hs && ct((*(types::Address*)(hs->mem().raw(addr))), i);
    }

    static bool classof(const ConditionInterface* ci) {
        return ci->ct == ConditionType::MEM_ADDR_CMP;
    }
};
class RegisterCompare : public ConditionInterface {
  public:
    isa::rf::RegisterClassType regtype;
    types::RegisterIndex idx;
    types::UnsignedInteger i;
    ComparisonType ct;

    RegisterCompare(
        isa::rf::RegisterClassType regtype,
        types::RegisterIndex idx,
        types::UnsignedInteger i,
        ComparisonType ct)
        : RegisterCompare(nullptr, regtype, idx, i, ct) {}
    RegisterCompare(
        hart::HartState* hs,
        isa::rf::RegisterClassType regtype,
        types::RegisterIndex idx,
        types::UnsignedInteger i,
        ComparisonType ct)
        : ConditionInterface(ConditionType::REG_CMP, hs), regtype(regtype),
          idx(idx), i(i), ct(ct) {}

    virtual ~RegisterCompare() = default;

    bool check() override {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            return hs && ct(types::Address(this->hs->rf().GPR.rawreg(idx)), i);
        }
        return false;
    }

    static bool classof(const ConditionInterface* ci) {
        return ci->ct == ConditionType::REG_CMP;
    }
};

class PCCompare : public ConditionInterface {
  public:
    types::SignedInteger offset;
    types::Address addr;
    ComparisonType ct;

    PCCompare(types::SignedInteger offset, types::Address addr, ComparisonType ct)
        : PCCompare(nullptr, offset, addr, ct) {}
    PCCompare(
        hart::HartState* hs,
        types::SignedInteger offset,
        types::Address addr,
        ComparisonType ct)
        : ConditionInterface(ConditionType::PC_CMP, hs), offset(offset),
          addr(addr), ct(ct) {}
    virtual ~PCCompare() = default;

    bool check() override {
        return hs && ct(types::Address(hs->pc + offset * 4), addr);
    }

    static bool classof(const ConditionInterface* ci) {
        return ci->ct == ConditionType::PC_CMP;
    }
};
class AlwaysTrue : public ConditionInterface {
  public:
    AlwaysTrue() : ConditionInterface(ConditionType::ALWAYS_TRUE) {}
    virtual ~AlwaysTrue() = default;

    bool check() override { return true; }

    static bool classof(const ConditionInterface* ci) {
        return ci->ct == ConditionType::ALWAYS_TRUE;
    }
};

} // namespace condition

struct ControlList;

// empty class for inheritance
class ControlBase {
  protected:
    bool useColor = false;

  public:
    virtual ~ControlBase() = default;
    virtual void setColor(bool useColor) { this->useColor = useColor; }
};

class Command : public ControlBase {
    friend ControlList;

  private:
    event::EventType et;
    std::vector<std::shared_ptr<action::ActionInterface>> actions;

  public:
    Command(
        event::EventType et,
        std::vector<std::shared_ptr<action::ActionInterface>> actions)
        : et(et), actions(actions) {}
    virtual ~Command() = default;
    virtual void doit(std::ostream* o = nullptr) {
        for(auto a : actions) {
            a->action(o);
        }
    }
    auto getEventType() { return et; }

    virtual void setColor(bool useColor) override {
        ControlBase::setColor(useColor);
        for(auto a : actions)
            a->setColor(useColor);
    }
};

class ConditionalCommand : public Command {
    friend ControlList;

  private:
    std::vector<std::shared_ptr<condition::ConditionInterface>> conditions;

  public:
    ConditionalCommand(
        event::EventType et,
        std::vector<std::shared_ptr<condition::ConditionInterface>> conditions,
        std::vector<std::shared_ptr<action::ActionInterface>> actions)
        : Command(et, actions), conditions(conditions) {}
    virtual ~ConditionalCommand() = default;
    virtual void doit(std::ostream* o = nullptr) override {
        bool shouldDoit = true;
        for(auto c : conditions) {
            shouldDoit = shouldDoit && c->check();
        }
        if(shouldDoit) Command::doit(o);
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
    virtual void setHS(hart::HartState* hs) {
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
    isa::rf::RegisterClassType regtype;
    types::RegisterIndex idx;

    WatchRegister(isa::rf::RegisterClassType regtype, types::RegisterIndex idx)
        : WatchRegister(nullptr, {}, regtype, idx) {}
    WatchRegister(
        hart::HartState* hs,
        std::vector<std::shared_ptr<action::ActionInterface>> actions,
        isa::rf::RegisterClassType regtype,
        types::RegisterIndex idx)
        : Watch(hs, actions), regtype(regtype), idx(idx) {}
    virtual ~WatchRegister() = default;

    virtual std::string name() override {
        return isa::rf::getRegisterClassString(this->regtype) + "[" +
               std::to_string(this->idx) + "]";
    }
    virtual std::optional<types::UnsignedInteger> readCurrentValue() override {
        if(hs) {
            if(this->regtype == isa::rf::RegisterClassType::GPR) {
                return this->hs->rf().GPR.rawreg(idx).get();
            }
        }
        return std::nullopt;
    }
};

class WatchMemoryAddress : public Watch {
  public:
    types::Address addr;

    WatchMemoryAddress(types::Address addr) : WatchMemoryAddress(nullptr, {}, addr) {}
    WatchMemoryAddress(
        hart::HartState* hs,
        std::vector<std::shared_ptr<action::ActionInterface>> actions,
        types::Address addr)
        : Watch(hs, actions), addr(addr) {}
    virtual ~WatchMemoryAddress() = default;

    virtual std::string name() override {
        std::stringstream ss;
        ss << "MEM[" << common::Format::doubleword << this->addr << "]";
        return ss.str();
    }
    virtual std::optional<types::UnsignedInteger> readCurrentValue() override {
        if(hs) {
            auto converted_addr = hs->mem().raw(addr);
            if(converted_addr) return *(types::Address*)(converted_addr);
        }
        return std::nullopt;
    }
};

struct ControlList {
    std::vector<std::shared_ptr<Command>> commands;
    std::vector<std::shared_ptr<Watch>> watches;
    ControlList(std::vector<std::shared_ptr<ControlBase>> controls) {
        for(auto control : controls) {
            if(auto cmd = std::dynamic_pointer_cast<Command>(control)) {
                commands.push_back(cmd);
            } else if(auto watch = std::dynamic_pointer_cast<Watch>(control)) {
                watches.push_back(watch);
            }
        }
    }

    std::vector<std::shared_ptr<action::ActionInterface>> allActions() {
        std::vector<std::shared_ptr<action::ActionInterface>> actions;
        for(auto c : commands) {
            actions.insert(actions.end(), c->actions.begin(), c->actions.end());
        }

        return actions;
    }
    std::vector<std::shared_ptr<condition::ConditionInterface>>
    allConditions() {
        std::vector<std::shared_ptr<condition::ConditionInterface>> conditions;
        for(auto c : commands) {
            if(std::shared_ptr<ConditionalCommand> ci =
                   std::dynamic_pointer_cast<ConditionalCommand>(c)) {
                conditions.insert(
                    conditions.end(),
                    ci->conditions.begin(),
                    ci->conditions.end());
            }
        }

        return conditions;
    }
    ControlList() = default;
    ControlList(const ControlList& other) = default;
    ControlList& operator=(const ControlList& other) = default;
};

} // namespace controller

#endif
