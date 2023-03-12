#ifndef ZIRCON_ISHELL_PARSER_EXPR_PARSER_H_
#define ZIRCON_ISHELL_PARSER_EXPR_PARSER_H_

#include "parser.h"

#include "command/expr.h"
#include "hart/types.h"

#include <memory>
#include <variant>
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
// expr -> MEM LBRACK expr RBRACK
// expr -> REGISTER
// expr -> NUM
// expr -> PC

class ExprParser {
  public:
    using StackElm = std::variant<Token, command::ExprPtr>;
    ExprParser(std::vector<Token> input) : input(input) {}
    command::ExprPtr parse();

    static bool isExprToken(const Token&);

  private:
    bool isTerminal(const StackElm&);
    bool isExpression(const StackElm&);
    const command::ExprPtr& getExpression(const StackElm& se);
    bool isTokenOfType(const StackElm&, TokenType);
    // sees through non terminal
    Token peekStack(const std::vector<StackElm>& stack);

    Token peekInput();
    Token getInputToken();

    bool isBinaryRule(const std::vector<StackElm>& rhs);
    command::ExprPtr reduceBinaryRule(const std::vector<StackElm>& rhs);
    bool isUnaryRule(const std::vector<StackElm>& rhs);
    command::ExprPtr reduceUnaryRule(const std::vector<StackElm>& rhs);
    bool isPrimaryRule(const std::vector<StackElm>& rhs);
    command::ExprPtr reducePrimaryRule(const std::vector<StackElm>& rhs);
    bool isParenRule(const std::vector<StackElm>& rhs);
    command::ExprPtr reduceParenRule(const std::vector<StackElm>& rhs);

    bool isValidRule(const std::vector<StackElm>& rhs);
    command::ExprPtr reduceRule(const std::vector<StackElm>& rhs);

  private:
    std::vector<Token> input;
};

} // namespace parser
} // namespace ishell

#endif
