#include "expr.h"

#include "hart/hartstate.h"

#include <assert.h>
#include <sstream>

namespace command {
types::SignedInteger Expr::eval(hart::HartState* hs) {
    if(hs == nullptr) return 0;
    return 0; // TODO: implement meco
}

std::string Expr::getString() {
    std::stringstream ss;
    switch(type) {
        case ExprType::BINARY:
            ss << left_expr->getString() << " " << this->getOperatorString()
               << " " << right_expr->getString();
            break;
        case ExprType::UNARY:
            ss << this->getOperatorString() << left_expr->getString();
            break;
        case ExprType::NUMBER: ss << std::to_string(number); break;
        case ExprType::REGISTER: ss << "$" << name; break;
        case ExprType::PC: ss << "$pc"; break;
        case ExprType::MEMORY:
            ss << "$mem[" << left_expr->getString() << "]";
            break;
    }
    return ss.str();
}

std::string Expr::getOperatorString() {
    assert(isBinary() || isUnary());
    switch(op_type) {
        case ExprOperatorType::MULTIPLY: return "*";
        case ExprOperatorType::DIVIDE: return "/";
        case ExprOperatorType::PLUS: return "+";
        case ExprOperatorType::MINUS: return "-";
        case ExprOperatorType::LSHIFT: return "<<";
        case ExprOperatorType::RSHIFT: return ">>";
        case ExprOperatorType::LT: return "<";
        case ExprOperatorType::LTEQ: return "<=";
        case ExprOperatorType::GT: return ">";
        case ExprOperatorType::GTEQ: return ">=";
        case ExprOperatorType::EQ: return "==";
        case ExprOperatorType::NEQ: return "!=";
        case ExprOperatorType::BW_AND: return "&";
        case ExprOperatorType::BW_OR: return "|";
        case ExprOperatorType::AND: return "&&";
        case ExprOperatorType::OR: return "|";
        case ExprOperatorType::NEGATE: return "-";
        case ExprOperatorType::BW_NOT: return "~";
        case ExprOperatorType::NOT: return "!";
        default: break;
    }
    return "ERR";
}

} // namespace command
