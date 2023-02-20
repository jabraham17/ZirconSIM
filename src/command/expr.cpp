#include "expr.h"

#include "common/debug.h"
#include "hart/hartstate.h"

#include <assert.h>
#include <sstream>

namespace command {

static types::SignedInteger apply_operator(
    types::SignedInteger lhs,
    ExprOperatorType op_type,
    types::SignedInteger rhs) {
    switch(op_type) {
        case ExprOperatorType::MULTIPLY: return lhs * rhs;
        case ExprOperatorType::DIVIDE: return lhs / rhs;
        case ExprOperatorType::PLUS: return lhs + rhs;
        case ExprOperatorType::MINUS: return lhs - rhs;
        case ExprOperatorType::LSHIFT: return lhs << rhs;
        case ExprOperatorType::RSHIFT: return lhs >> rhs;
        case ExprOperatorType::LT: return lhs < rhs;
        case ExprOperatorType::LTEQ: return lhs <= rhs;
        case ExprOperatorType::GT: return lhs > rhs;
        case ExprOperatorType::GTEQ: return lhs >= rhs;
        case ExprOperatorType::EQ: return lhs == rhs;
        case ExprOperatorType::NEQ: return lhs != rhs;
        case ExprOperatorType::BW_AND: return lhs & rhs;
        case ExprOperatorType::BW_OR: return lhs | rhs;
        case ExprOperatorType::AND: return lhs && rhs;
        case ExprOperatorType::OR: return lhs | rhs;
        default: break;
    }
    common::debug::logln(
        common::debug::DebugType::EXPR,
        "Attempted use of unsupported operator");
    return 0;
}
static types::SignedInteger
apply_operator(ExprOperatorType op_type, types::SignedInteger v) {
    switch(op_type) {
        case ExprOperatorType::NEGATE: return -v;
        case ExprOperatorType::BW_NOT: return ~v;
        case ExprOperatorType::NOT: return !v;
        default: break;
    }
    common::debug::logln(
        common::debug::DebugType::EXPR,
        "Attempted use of unsupported operator");
    return 0;
}

types::SignedInteger Expr::eval(hart::HartState* hs) {
    types::SignedInteger value = 0;
    if(hs == nullptr) {
        value = 0;
    } else {
        switch(type) {
            case ExprType::BINARY: {
                auto lhs = left_expr->eval(hs);
                auto rhs = right_expr->eval(hs);
                value = apply_operator(lhs, op_type, rhs);
                break;
            }
            case ExprType::UNARY: {
                auto v = left_expr->eval(hs);
                value = apply_operator(op_type, v);
                break;
            }
            case ExprType::NUMBER: value = number; break;
            case ExprType::REGISTER: {
                auto reg = hs->rf().getRegisterClassForType(register_.first);
                value = reg.rawreg(register_.second).get();
                break;
            }
            case ExprType::PC: value = hs->pc; break;
            case ExprType::MEMORY: {
                auto addr = left_expr->eval(hs);
                if(addr) {
                    auto converted_addr = hs->mem().raw(addr);
                    if(converted_addr) {
                        value = *(uint64_t*)(converted_addr);
                        break;
                    }
                }
                common::debug::logln(
                    common::debug::DebugType::EXPR,
                    "Unable to read memory due to invalid address");
                value = 0;
                break;
            }
            default: {
                common::debug::logln(
                    common::debug::DebugType::EXPR,
                    "Attempted use of unsupported expression");
                break;
            }
        }
    }
    common::debug::logln(
        common::debug::DebugType::EXPR,
        "eval(",
        this->getString(),
        ") = ",
        common::Format::dec,
        value);
    return value;
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
        case ExprType::REGISTER: ss << "$" << name_; break;
        case ExprType::PC: ss << "$pc"; break;
        case ExprType::MEMORY:
            ss << "$mem[" << left_expr->getString() << "]";
            break;
    }
    return ss.str();
}
std::string Expr::name() { return getString(); }

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
