
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
enum class ExprType { BINARY, UNARY, PAREN, NUMBER, REGISTER, PC, MEMORY };
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
  public:
    using PtrTy = std::shared_ptr<Expr>;

  private:
    ExprType type;

    // used for binary, unary, and paren
    ExprOperatorType op_type;
    PtrTy left_expr;
    PtrTy right_expr;

    // used for mem and num
    types::UnsignedInteger number;

    // registers use the original name in the name_ field used to
    // refer to it, since registers can have multiple names
    std::string name_;
    isa::rf::RegisterSymbol register_;

  private:
    // should be a private constructor, only allow creation through static make
    // methods since make_shared requires a public constructor, we have to use
    // this hacky PrivateTag to protect the public constructor another solution
    // was std::shared(new Expr), but make_shared provides some performance
    // benefits due to memory locality
    struct PrivateTag {};

  public:
    Expr(
        const PrivateTag&,
        ExprType type,
        ExprOperatorType op_type,
        PtrTy left_expr,
        PtrTy right_expr,
        types::UnsignedInteger number,
        std::string name_,
        isa::rf::RegisterSymbol register_)
        : type(type), op_type(op_type), left_expr(left_expr),
          right_expr(right_expr), number(number), name_(name_),
          register_(register_) {}

    static PtrTy
    makeBinaryExpression(PtrTy e1, ExprOperatorType op_type, PtrTy e2) {
        return std::make_shared<Expr>(
            PrivateTag{},
            ExprType::BINARY,
            op_type,
            e1,
            e2,
            types::UnsignedInteger{},
            std::string(),
            isa::rf::RegisterSymbol{});
    }
    static PtrTy makeUnaryExpression(ExprOperatorType op_type, PtrTy e) {
        return std::make_shared<Expr>(
            PrivateTag{},
            ExprType::UNARY,
            op_type,
            e,
            nullptr,
            types::UnsignedInteger{},
            std::string(),
            isa::rf::RegisterSymbol{});
    }
    static PtrTy makeParenExpression(PtrTy e) {
        return std::make_shared<Expr>(
            PrivateTag{},
            ExprType::PAREN,
            ExprOperatorType::NONE,
            e,
            nullptr,
            types::UnsignedInteger{},
            std::string(),
            isa::rf::RegisterSymbol{});
    }
    static PtrTy makeNumberExpression(types::UnsignedInteger number) {
        return std::make_shared<Expr>(
            PrivateTag{},
            ExprType::NUMBER,
            ExprOperatorType::NONE,
            nullptr,
            nullptr,
            number,
            std::string(),
            isa::rf::RegisterSymbol{});
    }
    static PtrTy makeRegisterExpression(
        std::string name_,
        isa::rf::RegisterSymbol register_) {
        return std::make_shared<Expr>(
            PrivateTag{},
            ExprType::REGISTER,
            ExprOperatorType::NONE,
            nullptr,
            nullptr,
            types::UnsignedInteger{},
            name_,
            register_);
    }
    static PtrTy makePCExpression() {
        return std::make_shared<Expr>(
            PrivateTag{},
            ExprType::PC,
            ExprOperatorType::NONE,
            nullptr,
            nullptr,
            types::UnsignedInteger{},
            std::string(),
            isa::rf::RegisterSymbol{});
    }

    static PtrTy makeMemoryExpression(PtrTy e) {
        return std::make_shared<Expr>(
            PrivateTag{},
            ExprType::MEMORY,
            ExprOperatorType::NONE,
            e,
            nullptr,
            types::UnsignedInteger{},
            std::string(),
            isa::rf::RegisterSymbol{});
    }
    Expr(const Expr&) = delete;            // copy construct
    Expr(Expr&&) = default;                // move construct
    Expr& operator=(const Expr&) = delete; // copy assignment
    Expr& operator=(Expr&&) = default;     // move assignment
    virtual ~Expr() = default;             // destructor

    ExprType getType() { return type; }
    bool isBinary() { return type == ExprType::BINARY; }
    bool isUnary() { return type == ExprType::UNARY; }
    bool isParen() { return type == ExprType::PAREN; }
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
