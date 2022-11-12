
#ifndef ZIRCON_CONTROLLER_COMMAND_H_
#define ZIRCON_CONTROLLER_COMMAND_H_

#include "cpu/cpu.h"
#include "cpu/isa/register.h"
#include "event/event.h"
#include "mem/memory-image.h"
#include <memory>

namespace controller {

using Address = uint64_t;
using RegisterIndex = uint64_t;

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
    ActionInterface(ActionType at = ActionType::NONE) : at(at) {}
    virtual ~ActionInterface() = default;
    void operator()(std::ostream* o = nullptr) { this->action(o); }
    virtual void action(std::ostream* o = nullptr) = 0;

    template <typename U> bool isa() {
        // assert(this && "isa<> used on a null type.");
        return U::classof(this);
    }
    template <typename U> U* cast() {
        // assert(isa<U>());
        return static_cast<U*>(this);
    }
};


// TODO: make the hs in the actioninterface, we always have it

class DumpRegisterClass : public ActionInterface {
  public:
    cpu::HartState* hs;
    isa::rf::RegisterClassType regtype;

  public:
    DumpRegisterClass(cpu::HartState* hs, isa::rf::RegisterClassType regtype)
        : ActionInterface(ActionType::DUMP_REG_CLASS), hs(hs), regtype(regtype) {}
    virtual ~DumpRegisterClass() = default;
    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_REG_CLASS;
    }
};
template <size_t NUM, size_t SIZE> class DumpRegister : public ActionInterface {
  public:
    RegisterClass<NUM, SIZE>* rc;
    RegisterIndex idx;

  public:
    DumpRegister(RegisterClass<NUM, SIZE>* rc, RegisterIndex idx)
        : ActionInterface(ActionType::DUMP_REG), rc(rc), idx(idx) {}
    virtual ~DumpRegister() = default;
    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_REG;
    }
};
class DumpPC : public ActionInterface {
  public:
    cpu::HartState* hs;

  public:
    DumpPC(cpu::HartState* hs) : ActionInterface(ActionType::DUMP_PC), hs(hs) {}
    virtual ~DumpPC() = default;
    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_PC;
    }
};
class DumpMemoryAddress : public ActionInterface {
  public:
    mem::MemoryImage* mi;
    Address addr;

  public:
    DumpMemoryAddress(mem::MemoryImage* mi, Address addr)
        : ActionInterface(ActionType::DUMP_MEM_ADDR), mi(mi), addr(addr) {}
    virtual ~DumpMemoryAddress() = default;
    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::DUMP_MEM_ADDR;
    }
};
class Stop : public ActionInterface {
  private:
    cpu::HartState* hs;

  public:
    Stop(cpu::HartState* hs) : ActionInterface(ActionType::STOP), hs(hs) {}
    virtual ~Stop() = default;
    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionInterface* ai) {
        return ai->at == ActionType::STOP;
    }
};
} // namespace action
// class Action_DUMP_REG : public {

// };

namespace condition {
enum class ConditionType {
    MEM_ADDR_EQ,
    REG_EQ,
    PC_EQ,
    ALWAYS_TRUE,
    NONE,
};

class ConditionInterface {
  public:
    ConditionType ct;
    ConditionInterface(ConditionType ct = ConditionType::NONE) : ct(ct) {}
    virtual ~ConditionInterface() = default;
    virtual bool check() = 0;
    template <typename U> bool isa() { return U::classof(this); }
    template <typename U> U* cast() { return static_cast<U*>(this); }
};
class PCEquals : public ConditionInterface {
  public:
    cpu::HartState* hs;
    Address addr;
    PCEquals(cpu::HartState* hs, Address addr)
        : ConditionInterface(ConditionType::PC_EQ), hs(hs), addr(addr) {}
    virtual ~PCEquals() = default;
    bool check() override { return hs && hs->pc == addr; }
    static bool classof(const ConditionInterface* ci) {
        return ci->ct == ConditionType::PC_EQ;
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

struct CommandList;

class Command {
    friend CommandList;

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
    friend CommandList;

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

struct CommandList {
    std::vector<std::shared_ptr<Command>> commands;
    CommandList(std::vector<std::shared_ptr<Command>> commands)
        : commands(commands) {}

    std::vector<std::shared_ptr<action::ActionInterface>> allActions() {
        std::vector<std::shared_ptr<action::ActionInterface>> actions;
        for(auto c : commands) {
            actions.insert(actions.end(), c->actions.begin(), c->actions.end());
        }

        return actions;
    }
        std::vector<std::shared_ptr<condition::ConditionInterface>> allConditions() {
        std::vector<std::shared_ptr<condition::ConditionInterface>> conditions;
        for(auto c : commands) {
            if(std::shared_ptr<ConditionalCommand> ci = std::dynamic_pointer_cast<ConditionalCommand>(c)) {
                conditions.insert(conditions.end(), ci->conditions.begin(), ci->conditions.end());
            }
        }

        return conditions;
    }
};

} // namespace controller

#endif
