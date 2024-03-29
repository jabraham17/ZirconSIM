#include "inst.h"

#include "color/color.h"
#include "common/utils.h"
#include "hart/syscall/syscall.h"

#include <sstream>

#include "instruction_match.h"

namespace isa {
namespace inst {
constexpr size_t Opcode::size() {
    return 1
#define R_TYPE(prefix, name, ...) +1
#define I_TYPE(prefix, name, ...) +1
#define S_TYPE(prefix, name, ...) +1
#define B_TYPE(prefix, name, ...) +1
#define U_TYPE(prefix, name, ...) +1
#define J_TYPE(prefix, name, ...) +1
#define CUSTOM(prefix, name, ...) +1
#include "defs/instructions.inc"
        ;
}

bool Opcode::isRType() const {
    switch(this->_value) {
        default: return false;
#define R_TYPE(prefix, name, ...)                                              \
    case Opcode::prefix##_##name: return true;
#include "defs/instructions.inc"
    }
}
bool Opcode::isIType() const {
    switch(this->_value) {
        default: return false;
#define I_TYPE(prefix, name, ...)                                              \
    case Opcode::prefix##_##name: return true;
#include "defs/instructions.inc"
    }
}
bool Opcode::isSType() const {
    switch(this->_value) {
        default: return false;
#define S_TYPE(prefix, name, ...)                                              \
    case Opcode::prefix##_##name: return true;
#include "defs/instructions.inc"
    }
}
bool Opcode::isBType() const {
    switch(this->_value) {
        default: return false;
#define B_TYPE(prefix, name, ...)                                              \
    case Opcode::prefix##_##name: return true;
#include "defs/instructions.inc"
    }
}
bool Opcode::isUType() const {
    switch(this->_value) {
        default: return false;
#define U_TYPE(prefix, name, ...)                                              \
    case Opcode::prefix##_##name: return true;
#include "defs/instructions.inc"
    }
}
bool Opcode::isJType() const {
    switch(this->_value) {
        default: return false;
#define J_TYPE(prefix, name, ...)                                              \
    case Opcode::prefix##_##name: return true;
#include "defs/instructions.inc"
    }
}
bool Opcode::isCustomType() const {
    switch(this->_value) {
        default: return false;
#define CUSTOM(prefix, name, ...)                                              \
    case Opcode::prefix##_##name: return true;
#include "defs/instructions.inc"
    }
}

int Opcode::getInstructionSize() const {
    switch(this->_value) {
        default: return 4;
    }
}

namespace internal {

std::string colorReset(bool doColor) {
    return doColor ? ::color::getReset() : "";
}
std::string colorError(bool doColor) {
    return doColor ? ::color::getColor({::color::ColorCode::RED}) : "";
}
std::string colorOpcode(bool doColor) {
    return doColor ? ::color::getColor({::color::ColorCode::ORANGE}) : "";
}
std::string colorReg(bool doColor) {
    return doColor ? ::color::getColor({::color::ColorCode::BLUE}) : "";
}
std::string colorNumber(bool doColor) {
    return doColor ? ::color::getColor({::color::ColorCode::PURPLE}) : "";
}

std::string OPCODE_NAME_TABLE[] = {
    "UNKNOWN",
#define R_TYPE(prefix, name, ...) #prefix "_" #name,
#define I_TYPE(prefix, name, ...) #prefix "_" #name,
#define S_TYPE(prefix, name, ...) #prefix "_" #name,
#define B_TYPE(prefix, name, ...) #prefix "_" #name,
#define U_TYPE(prefix, name, ...) #prefix "_" #name,
#define J_TYPE(prefix, name, ...) #prefix "_" #name,
#define CUSTOM(prefix, name, ...) #prefix "_" #name,
#include "defs/instructions.inc"
};
std::string OPCODE_NICE_NAME_TABLE[] = {
    "UNKNOWN",
#define R_TYPE(prefix, name, ...) #name,
#define I_TYPE(prefix, name, ...) #name,
#define S_TYPE(prefix, name, ...) #name,
#define B_TYPE(prefix, name, ...) #name,
#define U_TYPE(prefix, name, ...) #name,
#define J_TYPE(prefix, name, ...) #name,
#define CUSTOM(prefix, name, ...) #name,
#include "defs/instructions.inc"
};
std::string OPCODE_PREFIX_TABLE[] = {
    "UNKNOWN",
#define R_TYPE(prefix, ...) #prefix,
#define I_TYPE(prefix, ...) #prefix,
#define S_TYPE(prefix, ...) #prefix,
#define B_TYPE(prefix, ...) #prefix,
#define U_TYPE(prefix, ...) #prefix,
#define J_TYPE(prefix, ...) #prefix,
#define CUSTOM(prefix, ...) #prefix,
#include "defs/instructions.inc"
};

const std::string& getOpcodeNameFromTable(Opcode op) {
    return OPCODE_NAME_TABLE[op];
}
const std::string& getOpcodeNiceNameFromTable(Opcode op) {
    return OPCODE_NICE_NAME_TABLE[op];
}
const std::string& getPrefixFromTable(Opcode op) {
    return OPCODE_PREFIX_TABLE[op];
}

const Opcode getOpcodeFromTable(std::string argPrefix, std::string argName) {
#define CHECK_NAME_PREC(prefix, name)                                          \
    if(common::utils::tolower(#prefix) == common::utils::tolower(argPrefix) && \
       common::utils::tolower(#name) == common::utils::tolower(argName)) {     \
        return Opcode::prefix##_##name;                                        \
    }
#define R_TYPE(prefix, name, ...) CHECK_NAME_PREC(prefix, name)
#define I_TYPE(prefix, name, ...) CHECK_NAME_PREC(prefix, name)
#define S_TYPE(prefix, name, ...) CHECK_NAME_PREC(prefix, name)
#define B_TYPE(prefix, name, ...) CHECK_NAME_PREC(prefix, name)
#define U_TYPE(prefix, name, ...) CHECK_NAME_PREC(prefix, name)
#define J_TYPE(prefix, name, ...) CHECK_NAME_PREC(prefix, name)
#define CUSTOM(prefix, name, ...) CHECK_NAME_PREC(prefix, name)
#include "defs/instructions.inc"
#undef CHECK_NAME_PREC
    return Opcode::UNKNOWN;
}

uint64_t getOpcodeFieldFromTable(Opcode op) {
#define GET_OPCODE_FIELD(prefix, name, opcode)                                 \
    if(Opcode::prefix##_##name == op) {                                        \
        return opcode;                                                         \
    }
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    GET_OPCODE_FIELD(prefix, name, opcode)
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    GET_OPCODE_FIELD(prefix, name, opcode)
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    GET_OPCODE_FIELD(prefix, name, opcode)
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    GET_OPCODE_FIELD(prefix, name, opcode)
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    GET_OPCODE_FIELD(prefix, name, opcode)
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    GET_OPCODE_FIELD(prefix, name, opcode)
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    GET_OPCODE_FIELD(prefix, name, opcode)
#include "defs/instructions.inc"
#undef GET_OPCODE_FIELD
    return 0;
}
uint64_t getFunct7FieldFromTable(Opcode op) {
#define GET_FUNCT7_FIELD(prefix, name, funct7)                                 \
    if(Opcode::prefix##_##name == op) {                                        \
        return funct7;                                                         \
    }
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    GET_FUNCT7_FIELD(prefix, name, funct7)
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence) return 0;
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence) return 0;
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence) return 0;
#define U_TYPE(prefix, name, opcode, execution, precedence) return 0;
#define J_TYPE(prefix, name, opcode, execution, precedence) return 0;
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    return 0;
#include "defs/instructions.inc"
#undef GET_FUNCT7_FIELD
    return 0;
}
uint64_t getFunct3FieldFromTable(Opcode op) {
#define GET_FUNCT3_FIELD(prefix, name, funct3)                                 \
    if(Opcode::prefix##_##name == op) {                                        \
        return funct3;                                                         \
    }
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    GET_FUNCT3_FIELD(prefix, name, funct3)
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    GET_FUNCT3_FIELD(prefix, name, funct3)
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    GET_FUNCT3_FIELD(prefix, name, funct3)
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    GET_FUNCT3_FIELD(prefix, name, funct3)
#define U_TYPE(prefix, name, opcode, execution, precedence) return 0;
#define J_TYPE(prefix, name, opcode, execution, precedence) return 0;
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    return 0;
#include "defs/instructions.inc"
#undef GET_FUNCT3_FIELD
    return 0;
}

uint64_t OPCODE_PRECEDENCE[] = {
    0,
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    precedence,
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence) precedence,
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence) precedence,
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence) precedence,
#define U_TYPE(prefix, name, opcode, execution, precedence) precedence,
#define J_TYPE(prefix, name, opcode, execution, precedence) precedence,
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    precedence,
#include "defs/instructions.inc"
};

uint64_t getOpcodePrecedence(Opcode op) {
    return OPCODE_PRECEDENCE[uint64_t(op)];
}

#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    bool prefix##_##name##_matcher_func([[maybe_unused]] uint32_t bits) {      \
        do {                                                                   \
            matcher;                                                           \
        } while(0);                                                            \
    }
#include "defs/instructions.inc"

Opcode decodeInstruction(uint32_t bits) {
    Opcode matched = Opcode::UNKNOWN;
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct7(bits) == funct7 &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == Opcode::UNKNOWN ||                                          \
        precedence < getOpcodePrecedence(matched))) {                          \
        matched = Opcode::prefix##_##name;                                     \
    }
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == Opcode::UNKNOWN ||                                          \
        precedence < getOpcodePrecedence(matched))) {                          \
        matched = Opcode::prefix##_##name;                                     \
    }
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == Opcode::UNKNOWN ||                                          \
        precedence < getOpcodePrecedence(matched))) {                          \
        matched = Opcode::prefix##_##name;                                     \
    }
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == Opcode::UNKNOWN ||                                          \
        precedence < getOpcodePrecedence(matched))) {                          \
        matched = Opcode::prefix##_##name;                                     \
    }
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == Opcode::UNKNOWN ||                                          \
        precedence < getOpcodePrecedence(matched))) {                          \
        matched = Opcode::prefix##_##name;                                     \
    }
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == Opcode::UNKNOWN ||                                          \
        precedence < getOpcodePrecedence(matched))) {                          \
        matched = Opcode::prefix##_##name;                                     \
    }
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    if(prefix##_##name##_matcher_func(bits)) matched = Opcode::prefix##_##name;
#include "defs/instructions.inc"

    return matched;
}

// use precedence, rather than prefix, to distinguish
// this is SLOW
const Opcode getOpcodeFromTable(std::string arg) {
    Opcode matched = Opcode::UNKNOWN;
#define CHECK_NAME_PREC(prefix, name, precedence)                              \
    if(common::utils::tolower(#name) == common::utils::tolower(arg) &&         \
       (matched == Opcode::UNKNOWN ||                                          \
        precedence < getOpcodePrecedence(matched))) {                          \
        matched = Opcode::prefix##_##name;                                     \
    }
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    CHECK_NAME_PREC(prefix, name, precedence)
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    CHECK_NAME_PREC(prefix, name, precedence)
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    CHECK_NAME_PREC(prefix, name, precedence)
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    CHECK_NAME_PREC(prefix, name, precedence)
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    CHECK_NAME_PREC(prefix, name, precedence)
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    CHECK_NAME_PREC(prefix, name, precedence)
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    CHECK_NAME_PREC(prefix, name, precedence)
#include "defs/instructions.inc"
#undef CHECK_NAME_PREC
    return matched;
}

void executeInstruction(uint32_t bits, hart::HartState& hs) {
    Opcode opcode = decodeInstruction(bits);
    switch(opcode) {
        default: throw hart::IllegalInstructionException(bits);

#define SIMPLE_EXECUTION(prefix, name, execution)                              \
    case Opcode::prefix##_##name: do { execution;                              \
        } while(0);                                                            \
        break;
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    SIMPLE_EXECUTION(prefix, name, execution)
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    SIMPLE_EXECUTION(prefix, name, execution)
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    SIMPLE_EXECUTION(prefix, name, execution)
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    SIMPLE_EXECUTION(prefix, name, execution)
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    SIMPLE_EXECUTION(prefix, name, execution)
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    SIMPLE_EXECUTION(prefix, name, execution)
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    SIMPLE_EXECUTION(prefix, name, execution)
#include "defs/instructions.inc"
    }
}

#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    std::string prefix##_##name##_printer_func(                                \
        [[maybe_unused]] uint32_t bits,                                        \
        [[maybe_unused]] bool color = false) {                                 \
        do {                                                                   \
            printer;                                                           \
        } while(0);                                                            \
    }
#include "defs/instructions.inc"

std::string disassemble(uint32_t bits, uint32_t pc = 0, bool color = false) {
    Opcode op = decodeInstruction(bits);
    std::stringstream ss;

    switch(op) {
        default:
            ss << colorError(color) << "UNKNOWN[" << Opcode::getName(op) << "]"
               << colorReset(color);
            break;
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    case Opcode::prefix##_##name:                                              \
        ss << colorOpcode(color) << #name << colorReset(color) << " "          \
           << colorReg(color) << "x" << instruction::getRd(bits)               \
           << colorReset(color) << ", " << colorReg(color) << "x"              \
           << instruction::getRs1(bits) << colorReset(color) << ", "           \
           << colorReg(color) << "x" << instruction::getRs2(bits)              \
           << colorReset(color);                                               \
        break;
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case Opcode::prefix##_##name:                                              \
        ss << colorOpcode(color) << #name << colorReset(color) << " "          \
           << colorReg(color) << "x" << instruction::getRd(bits)               \
           << colorReset(color) << ", " << colorReg(color) << "x"              \
           << instruction::getRs1(bits) << colorReset(color) << ", "           \
           << colorNumber(color)                                               \
           << instruction::signext64<12>(instruction::getITypeImm(bits))       \
           << colorReset(color);                                               \
        break;
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case Opcode::prefix##_##name:                                              \
        ss << colorOpcode(color) << #name << colorReset(color) << " "          \
           << colorReg(color) << "x" << instruction::getRs2(bits)              \
           << colorReset(color) << ", " << colorNumber(color)                  \
           << instruction::signext64<12>(instruction::getSTypeImm(bits))       \
           << colorReset(color) << "(" << colorReg(color) << "x"               \
           << instruction::getRs1(bits) << colorReset(color) << ")";           \
        break;
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case Opcode::prefix##_##name:                                              \
        ss << colorOpcode(color) << #name << colorReset(color) << " "          \
           << colorReg(color) << "x" << instruction::getRs1(bits)              \
           << colorReset(color) << ", " << colorReg(color) << "x"              \
           << instruction::getRs2(bits) << colorReset(color) << ", "           \
           << colorNumber(color) << "0x" << std::hex                           \
           << (instruction::signext64<12>(instruction::getBTypeImm(bits)) +    \
               pc)                                                             \
           << colorReset(color);                                               \
        break;
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    case Opcode::prefix##_##name:                                              \
        ss << colorOpcode(color) << #name << colorReset(color) << " "          \
           << colorReg(color) << "x" << instruction::getRd(bits)               \
           << colorReset(color) << ", " << colorNumber(color)                  \
           << (instruction::getUTypeImm(bits) >> 12) << colorReset(color);     \
        break;
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    case Opcode::prefix##_##name:                                              \
        ss << colorOpcode(color) << #name << colorReset(color) << " "          \
           << colorReg(color) << "x" << instruction::getRd(bits)               \
           << colorReset(color) << ", " << colorNumber(color) << "0x"          \
           << std::hex                                                         \
           << (instruction::signext32<20>(instruction::getJTypeImm(bits)) +    \
               pc)                                                             \
           << colorReset(color);                                               \
        break;
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)  \
    case Opcode::prefix##_##name:                                              \
        ss << prefix##_##name##_printer_func(bits, color);                     \
        break;
#include "defs/instructions.inc"
    }
    return ss.str();
}

} // namespace internal

} // namespace inst
} // namespace isa
