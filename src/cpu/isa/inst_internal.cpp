#include "inst.h"
#include "instruction_match.h"
#include <sstream>

namespace isa {
namespace inst {
// namespace internal {
// static Opcode::ValueType op_counter = Opcode::UNKNOWN;
// }
// #define R_TYPE(prefix, name, ...) \
//     const Opcode::ValueType Opcode::prefix##_##name =                   \
//         ++internal::op_counter;
// #define I_TYPE(prefix, name, ...) \
//     const Opcode::ValueType Opcode::prefix##_##name =                   \
//         ++internal::op_counter;
// #define S_TYPE(prefix, name, ...) \
//     const Opcode::ValueType Opcode::prefix##_##name =                   \
//         ++internal::op_counter;
// #define B_TYPE(prefix, name, ...) \
//     const Opcode::ValueType Opcode::prefix##_##name =                   \
//         ++internal::op_counter;
// #define U_TYPE(prefix, name, ...) \
//     const Opcode::ValueType Opcode::prefix##_##name =                   \
//         ++internal::op_counter;
// #define J_TYPE(prefix, name, ...) \
//     const Opcode::ValueType Opcode::prefix##_##name =                   \
//         ++internal::op_counter;
// #define CUSTOM(prefix, name, ...) \
//     const Opcode::ValueType Opcode::prefix##_##name =                   \
//         ++internal::op_counter;
// #include "defs/instructions.inc"
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
    // return internal::op_counter;
}

namespace internal {

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

const std::string& getOpcodeNameFromTable(Opcode op) {
    return OPCODE_NAME_TABLE[op];
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
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence) precedence,
#include "defs/instructions.inc"
};

uint64_t getOpcodePrecedence(Opcode op) {
    return OPCODE_PRECEDENCE[uint64_t(op)];
}

#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
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
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
    if(prefix##_##name##_matcher_func(bits)) matched = Opcode::prefix##_##name;
#include "defs/instructions.inc"

    return matched;
}

void executeInstruction(uint32_t bits, cpu::HartState& hs) {
    Opcode op = decodeInstruction(bits);
    switch(op) {
        default: throw cpu::IllegalInstructionException();

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
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
    SIMPLE_EXECUTION(prefix, name, execution)
#include "defs/instructions.inc"
    }
}

std::string disassemble(uint32_t bits, uint32_t pc = 0) {
    Opcode op = decodeInstruction(bits);
    std::stringstream ss;
    switch(op) {
        default: ss << "UNKNOWN[" << Opcode::getName(op) << "]"; break;
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    case Opcode::prefix##_##name:                                              \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", x" << instruction::getRs2(bits); \
        break;
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case Opcode::prefix##_##name:                                              \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", "                                \
           << instruction::signext64<12>(instruction::getITypeImm(bits));      \
        break;
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case Opcode::prefix##_##name:                                              \
        ss << #name << " x" << instruction::getRs2(bits) << ", "               \
           << instruction::signext64<12>(instruction::getSTypeImm(bits))       \
           << "(x" << instruction::getRs1(bits) << ")";                        \
        break;
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case Opcode::prefix##_##name:                                              \
        ss << #name << " x" << instruction::getRs1(bits) << ", x"              \
           << instruction::getRs2(bits) << ", "                                \
           << "0x" << std::hex                                                 \
           << (instruction::signext64<12>(instruction::getBTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    case Opcode::prefix##_##name:                                              \
        ss << #name << " x" << instruction::getRd(bits) << ", "                \
           << (instruction::getUTypeImm(bits) >> 12);                          \
        break;
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    case Opcode::prefix##_##name:                                              \
        ss << #name << " x" << instruction::getRd(bits) << ", "                \
           << "0x" << std::hex                                                 \
           << (instruction::signext32<20>(instruction::getJTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
    case Opcode::prefix##_##name: ss << "custom_" #name; break;
#include "defs/instructions.inc"
    }
    return ss.str();
}
} // namespace internal

} // namespace inst
} // namespace isa
