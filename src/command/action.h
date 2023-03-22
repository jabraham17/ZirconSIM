#ifndef ZIRCON_COMMAND_ACTION_H_
#define ZIRCON_COMMAND_ACTION_H_

#include "command.h"
#include "expr.h"

#include "common/utils.h"
#include "hart/hart.h"
#include "hart/isa/register.h"
#include "hart/types.h"
#include "mem/memory-image.h"

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <variant>

namespace action {
enum class ActionType {
    STOP,
    PAUSE,
    RESUME,
    DISASM,
    DUMP,
    WATCH,
    SET,
    GROUP,
    NONE,
};
std::set<event::EventType>
getDefaultEventsForAction(ActionType at, ::command::CommandContext cc);

class ActionBase;
using ActionPtr = std::shared_ptr<ActionBase>;
class ActionBase {
  public:
    ActionType at;

  protected:
    hart::HartState* hs;
    size_t indent;
    bool useColor = false;

  public:
    ActionBase(ActionType at = ActionType::NONE, hart::HartState* hs = nullptr)
        : at(at), hs(hs), indent(0) {}
    virtual ~ActionBase() = default;

    void operator()(std::ostream* o = nullptr) { this->action(o); }
    virtual void action(std::ostream* o = nullptr) = 0;

    template <typename U> bool isa() { return U::classof(this); }
    template <typename U> U* cast() {
        assert(this->isa<U>());
        return static_cast<U*>(this);
    }
    ActionType getType() { return at; }
    virtual void setHS(hart::HartState* hs) { this->hs = hs; }
    virtual void increaseIndent(size_t indent = 2) { this->indent += indent; }
    // FIXME: possible sign underflow may occur here
    virtual void decreaseIndent(size_t indent = 2) { this->indent -= indent; }
    virtual void setColor(bool useColor) { this->useColor = useColor; }
    virtual std::string getString() { return "ACTION()"; }
};

#define MAKE_ACTION_0_ARGS(ClassName, ActionTypeName)                          \
    class ClassName : public ActionBase {                                      \
      public:                                                                  \
        ClassName() : ClassName(nullptr) {}                                    \
        ClassName(hart::HartState* hs)                                         \
            : ActionBase(ActionType::ActionTypeName, hs) {}                    \
        virtual ~ClassName() = default;                                        \
        void action(std::ostream* o = nullptr) override;                       \
        static bool classof(const ActionBase* ai) {                            \
            return ai->at == ActionType::ActionTypeName;                       \
        }                                                                      \
        virtual std::string getString() override { return #ClassName "()"; }   \
    };
#define MAKE_ACTION_1_ARGS(ClassName, ActionTypeName)                          \
    class ClassName : public ActionBase {                                      \
      private:                                                                 \
        command::ExprPtr expr;                                                 \
                                                                               \
      public:                                                                  \
        ClassName(command::ExprPtr expr) : ClassName(nullptr, expr) {}         \
        ClassName(hart::HartState* hs, command::ExprPtr expr)                  \
            : ActionBase(ActionType::ActionTypeName, hs), expr(expr) {}        \
        virtual ~ClassName() = default;                                        \
        void action(std::ostream* o = nullptr) override;                       \
        static bool classof(const ActionBase* ai) {                            \
            return ai->at == ActionType::ActionTypeName;                       \
        }                                                                      \
        virtual std::string getString() override {                             \
            return #ClassName "(" + expr->getString() + ")";                   \
        }                                                                      \
    };
#define MAKE_ACTION_2_ARGS(ClassName, ActionTypeName)                          \
    class ClassName : public ActionBase {                                      \
      private:                                                                 \
        command::ExprPtr expr1;                                                \
        command::ExprPtr expr2;                                                \
                                                                               \
      public:                                                                  \
        ClassName(command::ExprPtr expr1, command::ExprPtr expr2)              \
            : ClassName(nullptr, expr1, expr2) {}                              \
        ClassName(                                                             \
            hart::HartState* hs,                                               \
            command::ExprPtr expr1,                                            \
            command::ExprPtr expr2)                                            \
            : ActionBase(ActionType::ActionTypeName, hs), expr1(expr1),        \
              expr2(expr2) {}                                                  \
        virtual ~ClassName() = default;                                        \
        void action(std::ostream* o = nullptr) override;                       \
        static bool classof(const ActionBase* ai) {                            \
            return ai->at == ActionType::ActionTypeName;                       \
        }                                                                      \
        virtual std::string getString() override {                             \
            return #ClassName "(" + expr1->getString() + ", " +                \
                   expr2->getString() + ")";                                   \
        }                                                                      \
    };
#define MAKE_ACTION_VAR_ARGS(ClassName, ActionTypeName)                        \
    class ClassName : public ActionBase {                                      \
      private:                                                                 \
        std::vector<command::ExprPtr> expressions;                             \
                                                                               \
      public:                                                                  \
        template <class InputIt>                                               \
        ClassName(InputIt expressions_begin, InputIt expressions_end)          \
            : ClassName(nullptr, expressions_begin, expressions_end) {}        \
        template <class InputIt>                                               \
        ClassName(                                                             \
            hart::HartState* hs,                                               \
            InputIt expressions_begin,                                         \
            InputIt expressions_end)                                           \
            : ActionBase(ActionType::ActionTypeName, hs),                      \
              expressions(expressions_begin, expressions_end) {}               \
        virtual ~ClassName() = default;                                        \
        void action(std::ostream* o = nullptr) override;                       \
        static bool classof(const ActionBase* ai) {                            \
            return ai->at == ActionType::ActionTypeName;                       \
        }                                                                      \
        virtual std::string getString() override {                             \
            return #ClassName "(" +                                            \
                   common::utils::join(                                        \
                       expressions.begin(),                                    \
                       expressions.end(),                                      \
                       [](auto e) { return e->getString() }) +                 \
                   ")";                                                        \
        }                                                                      \
    };

MAKE_ACTION_0_ARGS(Stop, STOP)
MAKE_ACTION_0_ARGS(Pause, PAUSE)
MAKE_ACTION_0_ARGS(Resume, RESUME)
MAKE_ACTION_1_ARGS(Disasm, DISASM)
MAKE_ACTION_2_ARGS(Set, SET)

class Dump : public ActionBase {
  public:
    using DumpArg = std::variant<command::ExprPtr, std::string>;

  private:
    std::vector<DumpArg> args;

  public:
    template <class InputIt>
    Dump(InputIt args_begin, InputIt args_end)
        : Dump(nullptr, args_begin, args_end) {}
    template <class InputIt>
    Dump(hart::HartState* hs, InputIt args_begin, InputIt args_end)
        : ActionBase(ActionType::DUMP, hs), args(args_begin, args_end) {}
    virtual ~Dump() = default;
    void action(std::ostream* o = nullptr) override;
    static bool classof(const ActionBase* ai) {
        return ai->at == ActionType::DUMP;
    }
    virtual std::string getString() override {
        return "Dump(" +
               common::utils::join(
                   args.begin(),
                   args.end(),
                   [](auto a) {
                       if(std::holds_alternative<std::string>(a)) {
                           return "\"" + std::get<std::string>(a) + "\"";
                       } else {
                           return std::get<command::ExprPtr>(a)->getString();
                       }
                   }) +
               ")";
    }
};

class Watch : public ActionBase {
  private:
    command::ExprPtr expr;
    std::vector<ActionPtr> actions;
    std::optional<types::UnsignedInteger> previous;

  public:
    template <class InputIt>
    Watch(command::ExprPtr expr, InputIt actions_begin, InputIt actions_end)
        : Watch(nullptr, expr, actions_begin, actions_end) {}
    template <class InputIt>
    Watch(
        hart::HartState* hs,
        command::ExprPtr expr,
        InputIt actions_begin,
        InputIt actions_end)
        : ActionBase(ActionType::WATCH, hs), expr(expr),
          actions(actions_begin, actions_end), previous() {}
    virtual ~Watch() = default;
    void action(std::ostream* o = nullptr) override;
    static bool classof(const ActionBase* ai) {
        return ai->at == ActionType::WATCH;
    }
    virtual void setHS(hart::HartState* hs) override {
        ActionBase::setHS(hs);
        updateActions();
    }
    virtual void setColor(bool useColor) override {
        ActionBase::setColor(useColor);
        this->updateActions();
    }

    virtual void increaseIndent(size_t indent = 2) override {
        ActionBase::increaseIndent(indent);
        for(auto a : this->actions) {
            a->increaseIndent();
        }
    }
    virtual void decreaseIndent(size_t indent = 2) override {
        ActionBase::decreaseIndent(indent);
        for(auto a : this->actions) {
            a->decreaseIndent();
        }
    }
    virtual std::string getString() override {
        return "Dump(" + expr->getString() + " " +
               common::utils::join(
                   actions.begin(),
                   actions.end(),
                   [](auto a) { return a->getString(); }) +
               ")";
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

class ActionGroup : public ActionBase {
  private:
    std::vector<ActionPtr> actions;

  public:
    ActionGroup(std::vector<ActionPtr> actions)
        : ActionGroup(nullptr, actions) {}
    ActionGroup(hart::HartState* hs, std::vector<ActionPtr> actions)
        : ActionBase(ActionType::GROUP, hs), actions(actions) {}
    virtual ~ActionGroup() = default;

    void action(std::ostream* o = nullptr) override;

    static bool classof(const ActionBase* ai) {
        return ai->at == ActionType::GROUP;
    }
    virtual void setHS(hart::HartState* hs) override {
        ActionBase::setHS(hs);
        updateActions();
    }
    virtual void setColor(bool useColor) override {
        ActionBase::setColor(useColor);
        this->updateActions();
    }

    virtual void increaseIndent(size_t indent = 2) override {
        ActionBase::increaseIndent(indent);
        for(auto a : this->actions) {
            a->increaseIndent();
        }
    }
    virtual void decreaseIndent(size_t indent = 2) override {
        ActionBase::decreaseIndent(indent);
        for(auto a : this->actions) {
            a->decreaseIndent();
        }
    }
    std::vector<ActionPtr> getActions() { return actions; }

    virtual std::string getString() override {
        return "{" +
               common::utils::join(
                   actions.begin(),
                   actions.end(),
                   [](auto a) { return a->getString(); }) +
               "}";
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

#endif
