#include "expr_parser.h"

namespace ishell {
namespace parser {

enum class Precedence {
    // TAKE >
    // YIELD <
    // SAME =
    TAKE,
    YIELD,
    SAME,
    ERR,
    ACC
};
#include "parser/expr_parser_table.inc"
bool ExprParser::isTerminal(const StackElm& se) {
    return std::holds_alternative<Token>(se);
}
bool ExprParser::isExpression(const StackElm& se) {
    return std::holds_alternative<std::shared_ptr<command::Expr>>(se);
}
const std::shared_ptr<command::Expr>&
ExprParser::getExpression(const StackElm& se) {
    assert(isExpression(se));
    return std::get<std::shared_ptr<command::Expr>>(se);
}
bool ExprParser::isTokenOfType(const StackElm& se, TokenType tt) {
    return isTerminal(se) && std::get<Token>(se).token_type == tt;
}
// sees through non terminal
Token ExprParser::peekStack(const std::vector<StackElm>& stack) {
    for(auto it = stack.rbegin(); it != stack.rend(); it++) {
        if(isTerminal(*it)) {
            return std::get<Token>(*it);
        }
    }
    // no token, return error
    return Token();
}
Token ExprParser::peekInput() {
    if(input.empty()) return Token(TokenType::END_OF_FILE);
    else return input.front();
}
Token ExprParser::getInputToken() {
    if(input.empty()) return Token(TokenType::END_OF_FILE);
    else {
        auto t = input.front();
        input.erase(input.begin());
        return t;
    }
}

bool ExprParser::isBinaryRule(const std::vector<StackElm>& rhs) {
    if(rhs.size() == 3 && isExpression(rhs[0]) && isExpression(rhs[2]) &&
       isTerminal(rhs[1])) {
        auto tt = std::get<Token>(rhs[1]).token_type;
        switch(tt) {
            case TokenType::MULTIPLY:
            case TokenType::DIVIDE:
            case TokenType::PLUS:
            case TokenType::MINUS:
            case TokenType::LSHIFT:
            case TokenType::RSHIFT:
            case TokenType::LT:
            case TokenType::LTEQ:
            case TokenType::GT:
            case TokenType::GTEQ:
            case TokenType::EQ:
            case TokenType::NEQ:
            case TokenType::BW_AND:
            case TokenType::BW_OR:
            case TokenType::AND:
            case TokenType::OR: return true;
        }
    }
    return false;
}
std::shared_ptr<command::Expr>
ExprParser::reduceBinaryRule(const std::vector<StackElm>& rhs) {
    assert(isBinaryRule(rhs));
    auto op = command::ExprOperatorType::NONE;
    auto tt = std::get<Token>(rhs[1]).token_type;
    switch(tt) {
        case TokenType::MULTIPLY:
            op = command::ExprOperatorType::MULTIPLY;
            break;
        case TokenType::DIVIDE: op = command::ExprOperatorType::DIVIDE; break;
        case TokenType::PLUS: op = command::ExprOperatorType::PLUS; break;
        case TokenType::MINUS: op = command::ExprOperatorType::MINUS; break;
        case TokenType::LSHIFT: op = command::ExprOperatorType::LSHIFT; break;
        case TokenType::RSHIFT: op = command::ExprOperatorType::RSHIFT; break;
        case TokenType::LT: op = command::ExprOperatorType::LT; break;
        case TokenType::LTEQ: op = command::ExprOperatorType::LTEQ; break;
        case TokenType::GT: op = command::ExprOperatorType::GT; break;
        case TokenType::GTEQ: op = command::ExprOperatorType::GTEQ; break;
        case TokenType::EQ: op = command::ExprOperatorType::EQ; break;
        case TokenType::NEQ: op = command::ExprOperatorType::NEQ; break;
        case TokenType::BW_AND: op = command::ExprOperatorType::BW_AND; break;
        case TokenType::BW_OR: op = command::ExprOperatorType::BW_OR; break;
        case TokenType::AND: op = command::ExprOperatorType::AND; break;
        case TokenType::OR: op = command::ExprOperatorType::OR; break;
    }
    assert(op != command::ExprOperatorType::NONE);
    auto& left = getExpression(rhs[2]);
    auto& right = getExpression(rhs[0]);
    return std::make_shared<command::Expr>(
        std::move(left),
        op,
        std::move(right));
}

bool ExprParser::isUnaryRule(const std::vector<StackElm>& rhs) {
    if(rhs.size() == 2 && isTerminal(rhs[1]) && isExpression(rhs[0])) {
        auto tt = std::get<Token>(rhs[1]).token_type;
        switch(tt) {
            case TokenType::NEGATE:
            case TokenType::BW_NOT:
            case TokenType::NOT: return true;
        }
    }
    return false;
}
std::shared_ptr<command::Expr>
ExprParser::reduceUnaryRule(const std::vector<StackElm>& rhs) {
    assert(isUnaryRule(rhs));
    auto op = command::ExprOperatorType::NONE;
    auto tt = std::get<Token>(rhs[1]).token_type;
    switch(tt) {
        case TokenType::NEGATE: op = command::ExprOperatorType::NEGATE; break;
        case TokenType::BW_NOT: op = command::ExprOperatorType::BW_NOT; break;
        case TokenType::NOT: op = command::ExprOperatorType::NOT; break;
    }
    assert(op != command::ExprOperatorType::NONE);
    auto& e = getExpression(rhs[0]);
    return std::make_shared<command::Expr>(op, std::move(e));
}
bool ExprParser::isPrimaryRule(const std::vector<StackElm>& rhs) {
    if(rhs.size() == 1) {
        return isTokenOfType(rhs[0], TokenType::REGISTER) ||
               isTokenOfType(rhs[0], TokenType::NUM) ||
               isTokenOfType(rhs[0], TokenType::PC);
    } else if(rhs.size() == 4) {
        return isTokenOfType(rhs[3], TokenType::MEM) &&
               isTokenOfType(rhs[2], TokenType::LBRACK) &&
               isExpression(rhs[1]) && isTokenOfType(rhs[0], TokenType::RBRACK);
    } else return false;
}
std::shared_ptr<command::Expr>
ExprParser::reducePrimaryRule(const std::vector<StackElm>& rhs) {
    assert(isPrimaryRule(rhs));
    if(rhs.size() == 1) {
        if(isTokenOfType(rhs[0], TokenType::REGISTER)) {
            return std::make_shared<command::Expr>(
                std::get<Token>(rhs[0]).lexeme);
        } else if(isTokenOfType(rhs[0], TokenType::NUM)) {
            return std::make_shared<command::Expr>(
                types::strToUnsignedInteger(std::get<Token>(rhs[0]).lexeme));
        } else {
            return std::make_shared<command::Expr>(); // PC
        }
    } else {
        // memory
        auto& e = getExpression(rhs[1]);
        return std::make_shared<command::Expr>(std::move(e));
    }
}
bool ExprParser::isParenRule(const std::vector<StackElm>& rhs) {
    return rhs.size() == 3 && isTokenOfType(rhs[2], TokenType::LPAREN) &&
           isExpression(rhs[1]) && isTokenOfType(rhs[0], TokenType::RPAREN);
}
std::shared_ptr<command::Expr>
ExprParser::reduceParenRule(const std::vector<StackElm>& rhs) {
    assert(isParenRule(rhs));
    auto& e = getExpression(rhs[1]);
    return std::make_shared<command::Expr>(std::move(*e));
}

bool ExprParser::isValidRule(const std::vector<StackElm>& rhs) {
    return isBinaryRule(rhs) || isUnaryRule(rhs) || isPrimaryRule(rhs) ||
           isParenRule(rhs);
}

std::shared_ptr<command::Expr>
ExprParser::reduceRule(const std::vector<StackElm>& rhs) {
    assert(isValidRule(rhs));
    if(isBinaryRule(rhs)) {
        return reduceBinaryRule(rhs);
    } else if(isUnaryRule(rhs)) {
        return reduceUnaryRule(rhs);
    } else if(isPrimaryRule(rhs)) {
        return reducePrimaryRule(rhs);
    } else if(isParenRule(rhs)) {
        return reduceParenRule(rhs);
    } else {
        throw ParseException("Attempted to Reduce Invalid Rule");
    }
}

std::string getStringStackElm(ExprParser::StackElm e) {
if(std::holds_alternative<Token>(e)) {
            return std::get<Token>(e).getString() ;
        } else {
            return
                std::get<std::shared_ptr<command::Expr>>(e)->getString();
        }
}

void print_stack(std::vector<ExprParser::StackElm> s) {
    for(auto e : s) {
            std::cerr << getStringStackElm(e) << ", ";
    }
    std::cerr << "\n";
}

std::shared_ptr<command::Expr> ExprParser::parse() {
    std::vector<StackElm> stack;
    while(true) {
        auto toi = peekInput();
        auto tos = peekStack(stack);
        if(toi.token_type == TokenType::END_OF_FILE &&
           tos.token_type == TokenType::ERROR)
            break;
        auto action =
            precedence_table::getAction(tos.token_type, toi.token_type);

        if(action == Precedence::YIELD || action == Precedence::SAME) { // shift
            toi = getInputToken();
            stack.push_back(toi);
            // std::cerr << "AFTER SHIFT ";
            // print_stack(stack);
        } else if(action == Precedence::TAKE) {
            std::vector<StackElm> rhs;
            Token last_popped_term;
            last_popped_term.token_type = TokenType::ERROR;
            while(true) {
                // last of stack is added to rhs
                rhs.push_back(std::move(stack.back()));
                stack.pop_back();
                StackElm& stack_elm = rhs.back();

                if(isTerminal(stack_elm)) {
                    last_popped_term = std::get<Token>(stack_elm);
                }

                if(stack.empty()) break;
                if(isTerminal(stack.back()) &&
                   last_popped_term.token_type != TokenType::ERROR &&
                   precedence_table::getAction(
                       peekStack(stack).token_type,
                       last_popped_term.token_type) == Precedence::YIELD) {
                        // std::cerr << "TOS: " << peekStack(stack).getString() << " LAST POPPED: " << last_popped_term.getString() << "\n";
                    break;
                }
            }

            if(isValidRule(rhs)) {
                auto reduced = reduceRule(rhs);
                stack.push_back(std::move(reduced));
            } else {
                // print_stack(rhs);
                throw ParseException("No Valid Rule to Reduce");
            }
            // std::cerr << "AFTER TAKE ";
            // print_stack(stack);
        } else {
            throw ParseException("Invalid Table Action");
        }
    }
    if(stack.size() == 1 && isExpression(stack[0])) {
        auto& e = getExpression(stack[0]);
        // std::cerr << "DONE: " << e->getString() << "\n";
        return std::make_shared<command::Expr>(std::move(*e));
    } else {
        throw ParseException("Not a Single Expression Left");
    }
}

// considered a expr token if it is a valid in an expr
bool ExprParser::isExprToken(const Token& t) {
    switch(t.token_type) {
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::LSHIFT:
        case TokenType::RSHIFT:
        case TokenType::LT:
        case TokenType::LTEQ:
        case TokenType::GT:
        case TokenType::GTEQ:
        case TokenType::EQ:
        case TokenType::NEQ:
        case TokenType::BW_AND:
        case TokenType::BW_OR:
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::NEGATE:
        case TokenType::BW_NOT:
        case TokenType::NOT:
        case TokenType::LPAREN:
        case TokenType::RPAREN:
        case TokenType::MEM:
        case TokenType::LBRACK:
        case TokenType::RBRACK:
        case TokenType::REGISTER:
        case TokenType::NUM:
        case TokenType::PC: return true;
    }
    return false;
}

} // namespace parser
} // namespace ishell
