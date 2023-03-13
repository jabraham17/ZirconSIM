
#ifndef ZIRCON_COMMAND_EXPR_H_
#define ZIRCON_COMMAND_EXPR_H_

#include "common/debug.h"
#include "hart/isa/rf.h"
#include "hart/types.h"

#include <memory>
#include <string>
#include <variant>

namespace hart {
class HartState;
}

namespace command {
enum class ExprType {
    BINARY,
    UNARY,
    PAREN,
    NUMBER,
    REGISTER,
    PC,
    MEMORY,
    SYMBOL,
    NONE
};
enum class ExprOperatorType {
    NONE,
    MULTIPLY,
    DIVIDE,
    PLUS,
    MINUS,
    LSHIFT,
    RSHIFT,
    LT,
    LTEQ,
    GT,
    GTEQ,
    EQ,
    NEQ,
    BW_AND,
    BW_OR,
    AND,
    OR,
    NEGATE,
    BW_NOT,
    NOT
};
class Expr;
using ExprPtr = std::shared_ptr<Expr>;
class ParenExpr;

class Expr {
    friend ParenExpr;

  private:
    ExprType et;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const = 0;
    virtual bool setImpl(
        [[maybe_unused]] hart::HartState* hs,
        [[maybe_unused]] ExprPtr value) const {
        assert(false);
    }

  public:
    Expr(ExprType et = ExprType::NONE) : et(et) {}
    Expr(const Expr&) = delete; // delete copy
    Expr(Expr&&) = default;
    Expr& operator=(const Expr&) = delete; // delete copy
    Expr& operator=(Expr&&) = default;

    virtual std::string getString() const = 0;
    std::string getString() {
        return const_cast<const Expr*>(this)->getString();
    }
    explicit operator std::string() const { return this->getString(); }
    explicit operator std::string() {
        return std::string(*const_cast<const Expr*>(this));
    }

    types::SignedInteger eval(hart::HartState* hs) const {
        auto value = evalImpl(hs);
        common::debug::logln(
            common::debug::DebugType::EXPR,
            "eval(",
            this->getString(),
            ") = ",
            common::Format::dec,
            value);
        return value;
    };
    types::SignedInteger eval(hart::HartState* hs) {
        return const_cast<const Expr*>(this)->eval(hs);
    }
    types::SignedInteger operator()(hart::HartState* hs) const {
        return this->eval(hs);
    }
    types::SignedInteger operator()(hart::HartState* hs) {
        return (*const_cast<const Expr*>(this))(hs);
    }
    bool set(hart::HartState* hs, ExprPtr value) const {
        auto ret = setImpl(hs, value);
        common::debug::logln(
            common::debug::DebugType::EXPR,
            this->getString(),
            " = ",
            value->getString());
        return ret;
    };
    bool set(hart::HartState* hs, ExprPtr value) {
        return const_cast<const Expr*>(this)->set(hs, value);
    }

    ExprType getType() const { return et; }
    ExprType getType() { return const_cast<const Expr*>(this)->getType(); }

    virtual bool isLValue() const { return false; }

    template <typename U> bool isa() { return U::classof(this); }
    template <typename U> U* cast() {
        assert(this->isa<U>());
        return static_cast<U*>(this);
    }

    virtual ~Expr() = default;
};

class BinaryExpr : public Expr {
  private:
    ExprPtr lhs;
    ExprOperatorType op;
    ExprPtr rhs;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;

  public:
    BinaryExpr(ExprPtr lhs, ExprOperatorType op, ExprPtr rhs)
        : Expr(ExprType::BINARY), lhs(lhs), op(op), rhs(rhs) {}

    virtual std::string getString() const override;

    static bool classof(const Expr* e) {
        return e->getType() == ExprType::BINARY;
    }

    virtual ~BinaryExpr() = default;
};
class UnaryExpr : public Expr {
  private:
    ExprOperatorType op;
    ExprPtr expr;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;

  public:
    UnaryExpr(ExprOperatorType op, ExprPtr expr)
        : Expr(ExprType::UNARY), op(op), expr(expr) {}
    virtual ~UnaryExpr() = default;

    virtual std::string getString() const override;

    static bool classof(const Expr* e) {
        return e->getType() == ExprType::UNARY;
    }
};
class ParenExpr : public Expr {
  private:
    ExprPtr expr;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;
    virtual bool setImpl(hart::HartState* hs, ExprPtr value) const override {
        return this->expr->setImpl(hs, value);
    }

  public:
    ParenExpr(ExprPtr expr) : Expr(ExprType::PAREN), expr(expr) {}
    virtual ~ParenExpr() = default;

    virtual std::string getString() const override;

    virtual bool isLValue() const override { return this->expr->isLValue(); }

    static bool classof(const Expr* e) {
        return e->getType() == ExprType::PAREN;
    }
};
class NumberExpr : public Expr {
  private:
    types::UnsignedInteger number;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;

  public:
    NumberExpr(types::UnsignedInteger number)
        : Expr(ExprType::NUMBER), number(number) {}
    virtual ~NumberExpr() = default;

    virtual std::string getString() const override;

    static bool classof(const Expr* e) {
        return e->getType() == ExprType::NUMBER;
    }
};

class SymbolExpr : public Expr {
  private:
    std::string name;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;

  public:
    SymbolExpr(const std::string& name) : Expr(ExprType::SYMBOL), name(name) {}
    virtual ~SymbolExpr() = default;

    virtual std::string getString() const override;

    static bool classof(const Expr* e) {
        return e->getType() == ExprType::SYMBOL;
    }
};

class RegisterExpr : public Expr {
  private:
    std::string name;
    isa::rf::RegisterSymbol regSym;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;
    virtual bool setImpl(hart::HartState* hs, ExprPtr value) const override;

  public:
    RegisterExpr(std::string name, isa::rf::RegisterSymbol regSym)
        : Expr(ExprType::REGISTER), name(name), regSym(regSym) {}
    virtual ~RegisterExpr() = default;

    virtual std::string getString() const override;

    static bool classof(const Expr* e) {
        return e->getType() == ExprType::REGISTER;
    }
};
class PCExpr : public Expr {
  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;
    virtual bool setImpl(hart::HartState* hs, ExprPtr value) const override;

  public:
    PCExpr() : Expr(ExprType::PC) {}
    virtual ~PCExpr() = default;

    virtual std::string getString() const override;

    virtual bool isLValue() const override { return true; }

    static bool classof(const Expr* e) { return e->getType() == ExprType::PC; }
};
class MemoryExpr : public Expr {
  private:
    ExprPtr expr;

  protected:
    virtual types::SignedInteger evalImpl(hart::HartState* hs) const override;
    virtual bool setImpl(hart::HartState* hs, ExprPtr value) const override;

  public:
    MemoryExpr(ExprPtr expr) : Expr(ExprType::MEMORY), expr(expr) {}
    virtual ~MemoryExpr() = default;

    virtual std::string getString() const override;

    virtual bool isLValue() const override { return true; }

    static bool classof(const Expr* e) {
        return e->getType() == ExprType::MEMORY;
    }
};

} // namespace command

#endif
