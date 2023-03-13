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

std::string SymbolExpr::getString() const { return name; }
types::SignedInteger
SymbolExpr::evalImpl([[maybe_unused]] hart::HartState* hs) const {
    return 0;
#warning TODO: implement symbol table lookup
}

std::string RegisterExpr::getString() const { return "$" + name; }
types::SignedInteger RegisterExpr::evalImpl(hart::HartState* hs) const {
    return hs->rf()
        .getRegisterClassForType(regSym.rct)
        ->rawreg(regSym.idx)
        .get();
}
bool RegisterExpr::setImpl(hart::HartState* hs, ExprPtr value) const {
    auto& reg =
        hs->rf().getRegisterClassForType(regSym.rct)->rawreg(regSym.idx);
    auto v = value->eval(hs);
    reg.set(v);
    return true;
}

std::string PCExpr::getString() const { return "$PC"; }
types::SignedInteger PCExpr::evalImpl(hart::HartState* hs) const {
    return hs->pc;
}
bool PCExpr::setImpl(hart::HartState* hs, ExprPtr value) const {
    auto v = value->eval(hs);
    hs->setPC(v);
    return true;
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
bool MemoryExpr::setImpl(hart::HartState* hs, ExprPtr value) const {
    auto addr = expr->eval(hs);
    if(addr) {
        auto converted_addr = hs->mem().raw(addr);
        auto v = value->eval(hs);
        *(types::SignedInteger*)(converted_addr) = v;
    }
    common::debug::logln(
        common::debug::DebugType::EXPR,
        "Unable to set memory due to invalid address");
    return false;
}

} // namespace command
