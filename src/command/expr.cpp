#include "expr.h"

#include "hart/hartstate.h"

namespace command {

static types::SignedInteger applyOperator(
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
applyOperator(ExprOperatorType op_type, types::SignedInteger v) {
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

static std::string getOperatorString(ExprOperatorType op) {
    switch(op) {
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

std::string BinaryExpr::getString() const {
    return lhs->getString() + " " + getOperatorString(op) + " " +
           rhs->getString();
}
types::SignedInteger BinaryExpr::evalImpl(hart::HartState* hs) const {
    return applyOperator(lhs->eval(hs), op, rhs->eval(hs));
}

std::string UnaryExpr::getString() const {
    return getOperatorString(op) + expr->getString();
}
types::SignedInteger UnaryExpr::evalImpl(hart::HartState* hs) const {
    return applyOperator(op, expr->eval(hs));
}

std::string ParenExpr::getString() const {
    return "(" + expr->getString() + ")";
}
types::SignedInteger ParenExpr::evalImpl(hart::HartState* hs) const {
    return expr->eval(hs);
}

std::string NumberExpr::getString() const { return std::to_string(number); }
types::SignedInteger
NumberExpr::evalImpl([[maybe_unused]] hart::HartState* hs) const {
    return number;
}

std::string RegisterExpr::getString() const { return "$" + name; }
types::SignedInteger RegisterExpr::evalImpl(hart::HartState* hs) const {
    return hs->rf()
        .getRegisterClassForType(regSym.rct)
        .rawreg(regSym.idx)
        .get();
}

std::string PCExpr::getString() const { return "$PC"; }
types::SignedInteger PCExpr::evalImpl(hart::HartState* hs) const {
    return hs->pc;
}

std::string MemoryExpr::getString() const {
    return "$m[" + expr->getString() + "]";
}
types::SignedInteger MemoryExpr::evalImpl(hart::HartState* hs) const {
    auto addr = expr->eval(hs);
    if(addr) {
        auto converted_addr = hs->mem().raw(addr);
        if(converted_addr) return *(types::SignedInteger*)(converted_addr);
    }
    common::debug::logln(
        common::debug::DebugType::EXPR,
        "Unable to read memory due to invalid address");
    return 0;
}

// types::SignedInteger Expr::eval(hart::HartState* hs) {
//     types::SignedInteger value = 0;
//     if(hs == nullptr) {
//         value = 0;
//     } else {
//         switch(type) {
//             case ExprType::BINARY: {
//                 auto lhs = left_expr->eval(hs);
//                 auto rhs = right_expr->eval(hs);
//                 value = apply_operator(lhs, op_type, rhs);
//                 break;
//             }
//             case ExprType::UNARY: {
//                 auto v = left_expr->eval(hs);
//                 value = apply_operator(op_type, v);
//                 break;
//             }
//             case ExprType::PAREN: {
//                 value = left_expr->eval(hs);
//                 break;
//             }
//             case ExprType::NUMBER: value = number; break;
//             case ExprType::REGISTER: {
//                 auto reg = hs->rf().getRegisterClassForType(register_.rct);
//                 value = reg.rawreg(register_.idx).get();
//                 break;
//             }
//             case ExprType::PC: value = hs->pc; break;
//             case ExprType::MEMORY: {
//                 auto addr = left_expr->eval(hs);
//                 if(addr) {
//                     auto converted_addr = hs->mem().raw(addr);
//                     if(converted_addr) {
//                         value = *(uint64_t*)(converted_addr);
//                         break;
//                     }
//                 }
//                 common::debug::logln(
//                     common::debug::DebugType::EXPR,
//                     "Unable to read memory due to invalid address");
//                 value = 0;
//                 break;
//             }
//             default: {
//                 common::debug::logln(
//                     common::debug::DebugType::EXPR,
//                     "Attempted use of unsupported expression");
//                 break;
//             }
//         }
//     }
//     common::debug::logln(
//         common::debug::DebugType::EXPR,
//         "eval(",
//         this->getString(),
//         ") = ",
//         common::Format::dec,
//         value);
//     return value;
// }

// std::string Expr::getString() {
//     std::stringstream ss;
//     switch(type) {
//         case ExprType::BINARY:
//             ss << left_expr->getString() << " " << this->getOperatorString()
//                << " " << right_expr->getString();
//             break;
//         case ExprType::UNARY:
//             ss << this->getOperatorString() << left_expr->getString();
//             break;
//         case ExprType::PAREN: ss << "(" << left_expr->getString() << ")";
//         break; case ExprType::NUMBER: ss << std::to_string(number); break;
//         case ExprType::REGISTER: ss << "$" << name_; break;
//         case ExprType::PC: ss << "$pc"; break;
//         case ExprType::MEMORY:
//             ss << "$mem[" << left_expr->getString() << "]";
//             break;
//     }
//     return ss.str();
// }
// std::string Expr::name() { return getString(); }

// std::string Expr::getOperatorString() {
//     assert(isBinary() || isUnary());
//     switch(op_type) {
//         case ExprOperatorType::MULTIPLY: return "*";
//         case ExprOperatorType::DIVIDE: return "/";
//         case ExprOperatorType::PLUS: return "+";
//         case ExprOperatorType::MINUS: return "-";
//         case ExprOperatorType::LSHIFT: return "<<";
//         case ExprOperatorType::RSHIFT: return ">>";
//         case ExprOperatorType::LT: return "<";
//         case ExprOperatorType::LTEQ: return "<=";
//         case ExprOperatorType::GT: return ">";
//         case ExprOperatorType::GTEQ: return ">=";
//         case ExprOperatorType::EQ: return "==";
//         case ExprOperatorType::NEQ: return "!=";
//         case ExprOperatorType::BW_AND: return "&";
//         case ExprOperatorType::BW_OR: return "|";
//         case ExprOperatorType::AND: return "&&";
//         case ExprOperatorType::OR: return "|";
//         case ExprOperatorType::NEGATE: return "-";
//         case ExprOperatorType::BW_NOT: return "~";
//         case ExprOperatorType::NOT: return "!";
//         default: break;
//     }
//     return "ERR";
// }

} // namespace command
