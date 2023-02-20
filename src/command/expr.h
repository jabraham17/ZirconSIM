
#ifndef ZIRCON_COMMAND_EXPR_H_
#define ZIRCON_COMMAND_EXPR_H_

#include "hart/isa/rf.h"
#include "hart/types.h"

#include <memory>
#include <string>

namespace hart {
class HartState;
}

namespace command {
enum class ExprType { BINARY, UNARY, NUMBER, REGISTER, PC, MEMORY };
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

struct Expr {
  private:
    ExprType type;

    // used for binary and unary
    ExprOperatorType op_type;
    std::shared_ptr<Expr> left_expr;
    std::shared_ptr<Expr> right_expr;

    // used for mem and num
    types::UnsignedInteger number;

    // registers use the original name in the name_ field used to
    // refer to it, since registers can have multiple names
    std::string name_;
    isa::rf::RegisterSymbol register_;

  public:
    Expr(
        std::shared_ptr<Expr> e1,
        ExprOperatorType op_type,
        std::shared_ptr<Expr> e2)
        : type(ExprType::BINARY), op_type(op_type), left_expr(std::move(e1)),
          right_expr(std::move(e2)), number(), name_(), register_() {}
    Expr(ExprOperatorType op_type, std::shared_ptr<Expr> e1)
        : type(ExprType::UNARY), op_type(op_type), left_expr(std::move(e1)),
          right_expr(nullptr), number(), name_(), register_() {}
    Expr(types::UnsignedInteger number)
        : type(ExprType::NUMBER), op_type(ExprOperatorType::NONE),
          left_expr(nullptr), right_expr(nullptr), number(number), name_(),
          register_() {}
    Expr(std::string name_, isa::rf::RegisterSymbol register_)
        : type(ExprType::REGISTER), op_type(ExprOperatorType::NONE),
          left_expr(nullptr), right_expr(nullptr), number(), name_(name_),
          register_(register_) {}
    Expr()
        : type(ExprType::PC), op_type(ExprOperatorType::NONE),
          left_expr(nullptr), right_expr(nullptr), number(), name_(),
          register_() {}
    Expr(std::shared_ptr<Expr> address)
        : type(ExprType::MEMORY), op_type(ExprOperatorType::NONE),
          left_expr(std::move(address)), right_expr(nullptr), number(), name_(),
          register_() {}

    Expr(const Expr&) = delete;            // copy construct
    Expr(Expr&&) = default;                // move construct
    Expr& operator=(const Expr&) = delete; // copy assignment
    Expr& operator=(Expr&&) = default;     // move assignment
    virtual ~Expr() = default;             // destructor

    ExprType getType() { return type; }
    bool isBinary() { return type == ExprType::BINARY; }
    bool isUnary() { return type == ExprType::UNARY; }
    bool isNumber() { return type == ExprType::NUMBER; }
    bool isRegister() { return type == ExprType::REGISTER; }
    bool isPC() { return type == ExprType::PC; }
    bool isMemory() { return type == ExprType::MEMORY; }

    std::string getString();
    std::string name();

    types::SignedInteger eval(hart::HartState* hs = nullptr);

  private:
    std::string getOperatorString();
};
} // namespace command

#endif
