#ifndef ZIRCON_COMMAND_PARSER_EXPR_PARSER_H_
#define ZIRCON_COMMAND_PARSER_EXPR_PARSER_H_

#include "parser.h"

#include "hart/types.h"

#include <variant>
#include <memory>
#include <vector>

namespace ishell {

namespace parser {

// expr -> expr MULTIPLY expr
// expr -> expr DIVIDE expr
// expr -> expr PLUS expr
// expr -> expr MINUS expr
// expr -> expr LSHIFT expr
// expr -> expr RSHIFT expr
// expr -> expr LT expr
// expr -> expr LTEQ expr
// expr -> expr GT expr
// expr -> expr GTEQ expr
// expr -> expr EQ expr
// expr -> expr NEQ expr
// expr -> expr BW_AND expr
// expr -> expr BW_OR expr
// expr -> expr AND expr
// expr -> expr OR expr
// expr -> NEGATE expr
// expr -> BW_NOT expr
// expr -> NOT expr
// expr -> LPAREN expr RPAREN
// expr -> MEM LBRAC expr RBRAC
// expr -> REGISTER
// expr -> NUM
// expr -> PC
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
    ExprOperatorType op_type;
    std::unique_ptr<Expr> left_expr;
    std::unique_ptr<Expr> right_expr;
    std::string name;
    types::UnsignedInteger number;

  public:
    Expr(std::unique_ptr<Expr> e1, ExprOperatorType op_type, std::unique_ptr<Expr> e2)
        : type(ExprType::BINARY), op_type(op_type), left_expr(e1),
          right_expr(e2), name(), number() {}
    Expr(ExprOperatorType op_type, std::unique_ptr<Expr> e1)
        : type(ExprType::UNARY), op_type(op_type), left_expr(e1),
          right_expr(nullptr), name(), number() {}
    Expr(types::UnsignedInteger number)
        : type(ExprType::NUMBER), op_type(ExprOperatorType::NONE),
          left_expr(nullptr), right_expr(nullptr), name(), number(number) {}
    Expr(std::string reg_name)
        : type(ExprType::REGISTER), op_type(ExprOperatorType::NONE),
          left_expr(nullptr), right_expr(nullptr), name(reg_name), number() {}
    Expr()
        : type(ExprType::PC), op_type(ExprOperatorType::NONE),
          left_expr(nullptr), right_expr(nullptr), name(), number() {}
    Expr(std::unique_ptr<Expr> address)
        : type(ExprType::MEMORY), op_type(ExprOperatorType::NONE),
          left_expr(address), right_expr(nullptr), name(), number() {}

    ExprType getType() { return type; }
    bool isBinary() { return type == ExprType::BINARY; }
    bool isUnary() { return type == ExprType::UNARY; }
    bool isNumber() { return type == ExprType::NUMBER; }
    bool isRegister() { return type == ExprType::REGISTER; }
    bool isPC() { return type == ExprType::PC; }
    bool isMemory() { return type == ExprType::MEMORY; }
};

class ExprParser {
  public:
    using StackElm = std::variant<Token, std::unique_ptr<Expr>>;
    ExprParser(std::vector<Token> input) : input(input), stack() {}
    std::unique_ptr<Expr> parse();

  private:
    bool isTerminal(const StackElm&);
    bool isExpression(const StackElm&);
    bool isTokenOfType(const StackElm&, TokenType);
    // sees through non terminal
    Token peekStack(const std::vector<StackElm>& stack);

    Token peekInput();
    Token getInputToken();

    bool isBinaryRule(const std::vector<StackElm>& rhs);
    std::unique_ptr<Expr> reduceBinaryRule(const std::vector<StackElm>& rhs);
    bool isUnaryRule(const std::vector<StackElm>& rhs);
    std::unique_ptr<Expr> reduceUnaryRule(const std::vector<StackElm>& rhs);
    bool isPrimaryRule(const std::vector<StackElm>& rhs);
    std::unique_ptr<Expr> reducePrimaryRule(const std::vector<StackElm>& rhs);
    bool isParenRule(const std::vector<StackElm>& rhs);
    std::unique_ptr<Expr> reduceParenRule(const std::vector<StackElm>& rhs);

    bool isValidRule(const std::vector<StackElm>& rhs);
    std::unique_ptr<Expr> reduceRule(const std::vector<StackElm>& rhs);

  private:
    std::vector<Token> input;
};

} // namespace parser
} // namespace ishell

#endif
