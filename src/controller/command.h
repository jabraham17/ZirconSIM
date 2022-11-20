
#ifndef ZIRCON_CONTROLLER_COMMAND_H_
#define ZIRCON_CONTROLLER_COMMAND_H_

#include "cpu/cpu.h"
#include "cpu/isa/register.h"
#include "event/event.h"
#include "mem/memory-image.h"
#include <memory>
#include <optional>

namespace controller {

using Address = uint64_t;
using RegisterIndex = uint64_t;
using Integer = uint64_t;

namespace action {
enum class ActionType {
    DUMP_REG,
    DUMP_REG_CLASS,
    DUMP_MEM_ADDR,
    DUMP_PC,
    STOP,
    NONE,
};
class ActionInterface {
  public:
    ActionType at;
    cpu::HartState* hs;

    ActionInterface(
        ActionType at = ActionType::NONE,
        cpu::HartState* hs = nullptr)
        : at(at), hs(hs) {}
    virtual ~ActionInterface() = default;

    void operator()(std::ostream* o = nullptr) { this->action(o); }
    virtual void action(std::ostream* o = nullptr) = 0;

    template <typename U> bool isa() { return U::classof(this); }
    template <typename U> U* cast() {
        assert(this->isa<U>());
        return static_cast<U*>(this);
    }
};

class DumpRegisterClass : public ActionInterface {
  public:
    isa::rf::RegisterClassType regtype;

    DumpRegisterClass(isa::rf::RegisterClassType regtype)
        : DumpRegisterClass(nullptr, regtype) {}
    DumpRegisterClass(cpu::HartState* hs, isa::rf::RegisterClassType regtype)
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
    RegisterIndex idx;

    DumpRegister(isa::rf::RegisterClassType regtype, RegisterIndex idx)
        : DumpRegister(nullptr, regtype, idx) {}
    DumpRegister(
        cpu::HartState* hs,
        isa::rf::RegisterClassType regtype,
        RegisterIndex idx)
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
    DumpPC() : DumpPC(nullptr) {}
    DumpPC(cpu::HartState* hs) : ActionInterface(ActionType::DUMP_PC, hs) {}
    virtual ~DumpPC() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_PC;
    }
};
class DumpMemoryAddress : public ActionInterface {
  public:
    Address addr;

    DumpMemoryAddress(Address addr) : DumpMemoryAddress(nullptr, addr) {}
    DumpMemoryAddress(cpu::HartState* hs, Address addr)
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
    Stop(cpu::HartState* hs) : ActionInterface(ActionType::STOP, hs) {}
    virtual ~Stop() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::STOP;
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
    cpu::HartState* hs;

    ConditionInterface(
        ConditionType ct = ConditionType::NONE,
        cpu::HartState* hs = nullptr)
        : ct(ct), hs(hs) {}
    virtual ~ConditionInterface() = default;

    virtual bool check() = 0;

    template <typename U> bool isa() { return U::classof(this); }
    template <typename U> U* cast() {
        assert(this->isa<U>());
        return static_cast<U*>(this);
    }
};
class MemAddrCompare : public ConditionInterface {
  public:
    Address addr;
    Integer i;
    ComparisonType ct;

    MemAddrCompare(Address addr, Integer i, ComparisonType ct)
        : MemAddrCompare(nullptr, addr, i, ct) {}
    MemAddrCompare(
        cpu::HartState* hs,
        Address addr,
        Integer i,
        ComparisonType ct)
        : ConditionInterface(ConditionType::MEM_ADDR_CMP, hs), addr(addr), i(i),
          ct(ct) {}
    virtual ~MemAddrCompare() = default;

    bool check() override {
        return hs && ct((*(uint64_t*)(hs->memimg.raw(addr))), i);
    }

    static bool classof(const ConditionInterface* ci) {
        return ci->ct == ConditionType::MEM_ADDR_CMP;
    }
};
class RegisterCompare : public ConditionInterface {
  public:
    isa::rf::RegisterClassType regtype;
    RegisterIndex idx;
    Integer i;
    ComparisonType ct;

    RegisterCompare(
        isa::rf::RegisterClassType regtype,
        RegisterIndex idx,
        Integer i,
        ComparisonType ct)
        : RegisterCompare(nullptr, regtype, idx, i, ct) {}
    RegisterCompare(
        cpu::HartState* hs,
        isa::rf::RegisterClassType regtype,
        RegisterIndex idx,
        Integer i,
        ComparisonType ct)
        : ConditionInterface(ConditionType::REG_CMP, hs), regtype(regtype),
          idx(idx), i(i), ct(ct) {}

    virtual ~RegisterCompare() = default;

    bool check() override {
        if(this->regtype == isa::rf::RegisterClassType::GPR) {
            return hs && ct(uint64_t(this->hs->rf.GPR.rawreg(idx)), i);
        }
        return false;
    }

    static bool classof(const ConditionInterface* ci) {
        return ci->ct == ConditionType::REG_CMP;
    }
};

class PCCompare : public ConditionInterface {
  public:
    Address addr;
    ComparisonType ct;

    PCCompare(Address addr, ComparisonType ct) : PCCompare(nullptr, addr, ct) {}
    PCCompare(cpu::HartState* hs, Address addr, ComparisonType ct)
        : ConditionInterface(ConditionType::PC_CMP, hs), addr(addr), ct(ct) {}
    virtual ~PCCompare() = default;

    bool check() override { return hs && ct(uint64_t(hs->pc), addr); }

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
  public:
    virtual ~ControlBase() = default;
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


// chnage watches to define commands to also dump
// maybe restructure watches as a new operator?
// it becomes part of the conidition?

class Watch : public ControlBase {
  private:
    std::optional<uint64_t> previous;
    std::ostream* out;

  public:
    cpu::HartState* hs;

    Watch(cpu::HartState* hs = nullptr) : previous(), out(nullptr), hs(hs) {}
    virtual ~Watch() = default;

    void setLog(std::ostream* o) { this->out = o; }

    virtual bool hasChanged() {
        std::optional<uint64_t> value = readCurrentValue();
        // no current value, no change
        if(!value.has_value()) return false;
        // no previous value, no change
        if(!previous.has_value()) return false;

        return *value != *previous;
    }
    virtual void update() {
        std::optional<uint64_t> current = readCurrentValue();
        if(!current.has_value()) return;
        // no value, read one
        if(!previous.has_value()) {
            if(out)
                *out << "WATCH " << name() << ": Setting initial value to "
                     << common::Format::doubleword << *current << std::endl;
            previous = *current;
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
        }
    }
    virtual std::string name() = 0;
    virtual std::optional<uint64_t> readCurrentValue() = 0;
};

class WatchRegister : public Watch {
  public:
    isa::rf::RegisterClassType regtype;
    RegisterIndex idx;

    WatchRegister(isa::rf::RegisterClassType regtype, RegisterIndex idx)
        : WatchRegister(nullptr, regtype, idx) {}
    WatchRegister(
        cpu::HartState* hs,
        isa::rf::RegisterClassType regtype,
        RegisterIndex idx)
        : Watch(hs), regtype(regtype), idx(idx) {}
    virtual ~WatchRegister() = default;

    std::string name() {
        return isa::rf::getRegisterClassString(this->regtype) + "[" +
               std::to_string(this->idx) + "]";
    }
    std::optional<uint64_t> readCurrentValue() {
        if(hs) {
            if(this->regtype == isa::rf::RegisterClassType::GPR) {
                return this->hs->rf.GPR.rawreg(idx).get();
            }
        }
        return std::nullopt;
    }
};

class WatchMemoryAddress : public Watch {
  public:
    Address addr;

    WatchMemoryAddress(Address addr) : WatchMemoryAddress(nullptr, addr) {}
    WatchMemoryAddress(cpu::HartState* hs, Address addr)
        : Watch(hs), addr(addr) {}
    virtual ~WatchMemoryAddress() = default;

    std::string name() {
        std::stringstream ss;
        ss << "MEM[" << common::Format::doubleword << this->addr << "]";
        return ss.str();
    }
    std::optional<uint64_t> readCurrentValue() {
        if(hs) {
            auto converted_addr = hs->memimg.raw(addr);
            if(converted_addr) return *(uint64_t*)(converted_addr);
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
};

} // namespace controller

#endif
