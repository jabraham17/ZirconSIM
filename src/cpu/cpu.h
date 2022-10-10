
#ifndef ZIRCON_CPU_CPU_H_
#define ZIRCON_CPU_CPU_H_

#include "isa/instruction_match.h"
#include "mem/memory-image.h"
#include "register.h"
#include "trace/trace.h"

#include <string>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace cpu {

class Hart64State {
  public:
#define REG_CASE(...) MAKE_REGISTER(__VA_ARGS__),
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    RegisterClass<number_regs, reg_size> classname##_RF = {                    \
        #classname,                                                            \
        #reg_prefix,                                                           \
        {REGISTER_CLASS_##classname(REG_CASE)}};
#include "isa/defs/registers.inc"

    mem::MemoryImage& memimg;
    uint64_t pc; // address in memory of current instruction
    uint32_t getInstWord() { return memimg.word(pc); }

    Hart64State(mem::MemoryImage& m, TraceMode tm = TraceMode::NONE) : memimg(m) {
        #define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size) \
        classname##_RF.setTraceMode(tm);
        #include "isa/defs/registers.inc"
    }
};

struct CPUException : public std::exception {
    const char* what() const throw() { return "CPU Exception"; }
};
struct IllegalInstructionException : public CPUException {
    const char* what() const throw() { return "Illegal Instruction"; }
};

namespace isa {

enum class ISA : uint64_t {
    FIRST_OP,
    UNKNOWN,
    FIRST_RV32I_OP,
#define R_TYPE(name, ...) rv32i_##name,
#define I_TYPE(name, ...) rv32i_##name,
#define S_TYPE(name, ...) rv32i_##name,
#define B_TYPE(name, ...) rv32i_##name,
#define U_TYPE(name, ...) rv32i_##name,
#define J_TYPE(name, ...) rv32i_##name,
#define CUSTOM(name, ...) rv32i_##name,
#include "isa/rv32i.inc"
    LAST_RV32I_OP,
    FIRST_RV64I_OP,
#define R_TYPE(name, ...) rv64i_##name,
#define I_TYPE(name, ...) rv64i_##name,
#define S_TYPE(name, ...) rv64i_##name,
#define B_TYPE(name, ...) rv64i_##name,
#define U_TYPE(name, ...) rv64i_##name,
#define J_TYPE(name, ...) rv64i_##name,
#define CUSTOM(name, ...) rv64i_##name,
#include "isa/rv64i.inc"
    LAST_RV64I_OP,
    LAST_OP
};

std::string
    ISA_NAME_TABLE[uint64_t(ISA::LAST_OP) - uint64_t(ISA::FIRST_OP) + 1] = {
        "FIRST_OP",
        "UNKNOWN",
        "FIRST_RV32I_OP",
#define R_TYPE(name, ...) "rv32i_" #name,
#define I_TYPE(name, ...) "rv32i_" #name,
#define S_TYPE(name, ...) "rv32i_" #name,
#define B_TYPE(name, ...) "rv32i_" #name,
#define U_TYPE(name, ...) "rv32i_" #name,
#define J_TYPE(name, ...) "rv32i_" #name,
#define CUSTOM(name, ...) "rv32i_" #name,
#include "isa/rv32i.inc"
        "LAST_RV32I_OP",
        "FIRST_RV64I_OP",
#define R_TYPE(name, ...) "rv64i_" #name,
#define I_TYPE(name, ...) "rv64i_" #name,
#define S_TYPE(name, ...) "rv64i_" #name,
#define B_TYPE(name, ...) "rv64i_" #name,
#define U_TYPE(name, ...) "rv64i_" #name,
#define J_TYPE(name, ...) "rv64i_" #name,
#define CUSTOM(name, ...) "rv64i_" #name,
#include "isa/rv64i.inc"
        "LAST_RV64I_OP",
        "LAST_OP"};

std::string getISAOpName(ISA op) { return ISA_NAME_TABLE[uint64_t(op)]; }

uint64_t ISA_PRECEDENCE[uint64_t(ISA::LAST_OP) - uint64_t(ISA::FIRST_OP) + 1] =
    {0,
     0,
     0,
#define R_TYPE(name, opcode, funct7, funct3, execution, precedence) precedence,
#define I_TYPE(name, opcode, funct3, execution, precedence) precedence,
#define S_TYPE(name, opcode, funct3, execution, precedence) precedence,
#define B_TYPE(name, opcode, funct3, execution, precedence) precedence,
#define U_TYPE(name, opcode, execution, precedence) precedence,
#define J_TYPE(name, opcode, execution, precedence) precedence,
#define CUSTOM(name, opcode, matcher, execution, precedence) precedence,
#include "isa/rv32i.inc"
     0,
     0,
#define R_TYPE(name, opcode, funct7, funct3, execution, precedence) precedence,
#define I_TYPE(name, opcode, funct3, execution, precedence) precedence,
#define S_TYPE(name, opcode, funct3, execution, precedence) precedence,
#define B_TYPE(name, opcode, funct3, execution, precedence) precedence,
#define U_TYPE(name, opcode, execution, precedence) precedence,
#define J_TYPE(name, opcode, execution, precedence) precedence,
#define CUSTOM(name, opcode, matcher, execution, precedence) precedence,
#include "isa/rv64i.inc"
     0,
     0};

#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    bool rv32i_##name##_matcher_func([[maybe_unused]] uint32_t bits) {         \
        do {                                                                   \
            matcher;                                                           \
        } while(0);                                                            \
    }
#include "isa/rv32i.inc"
#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    bool rv64i_##name##_matcher_func([[maybe_unused]] uint32_t bits) {         \
        do {                                                                   \
            matcher;                                                           \
        } while(0);                                                            \
    }
#include "isa/rv64i.inc"

uint64_t getISAPrecedence(ISA op) { return ISA_PRECEDENCE[uint64_t(op)]; }

ISA decodeInstruction(uint32_t bits) {
    ISA matched = ISA::UNKNOWN;
#define R_TYPE(name, opcode, funct7, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct7(bits) == funct7 &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv32i_##name;                                           \
    }
#define I_TYPE(name, opcode, funct3, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv32i_##name;                                           \
    }
#define S_TYPE(name, opcode, funct3, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv32i_##name;                                           \
    }
#define B_TYPE(name, opcode, funct3, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv32i_##name;                                           \
    }
#define U_TYPE(name, opcode, execution, precedence)                            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv32i_##name;                                           \
    }
#define J_TYPE(name, opcode, execution, precedence)                            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv32i_##name;                                           \
    }
#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    if(rv32i_##name##_matcher_func(bits)) matched = ISA::rv32i_##name;
#include "isa/rv32i.inc"

#define R_TYPE(name, opcode, funct7, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct7(bits) == funct7 &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv64i_##name;                                           \
    }
#define I_TYPE(name, opcode, funct3, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv64i_##name;                                           \
    }
#define S_TYPE(name, opcode, funct3, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv64i_##name;                                           \
    }
#define B_TYPE(name, opcode, funct3, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv64i_##name;                                           \
    }
#define U_TYPE(name, opcode, execution, precedence)                            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv64i_##name;                                           \
    }
#define J_TYPE(name, opcode, execution, precedence)                            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::rv64i_##name;                                           \
    }
#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    if(rv64i_##name##_matcher_func(bits)) matched = ISA::rv64i_##name;
#include "isa/rv64i.inc"

    return matched;
}

void executeInstruction(uint32_t bits, Hart64State& hs) {
    ISA op = decodeInstruction(bits);
    switch(op) {
        default: throw IllegalInstructionException();

#define R_TYPE(name, opcode, funct7, funct3, execution, precedence)            \
    case ISA::rv32i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define I_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv32i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define S_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv32i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define B_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv32i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define U_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv32i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define J_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv32i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    case ISA::rv32i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#include "isa/rv32i.inc"

#define R_TYPE(name, opcode, funct7, funct3, execution, precedence)            \
    case ISA::rv64i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define I_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv64i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define S_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv64i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define B_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv64i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define U_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv64i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define J_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv64i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    case ISA::rv64i_##name: do { execution;                                    \
        } while(0);                                                            \
        break;
#include "isa/rv64i.inc"
    }
}

std::string disassemble(uint32_t bits, uint32_t pc = 0) {
    ISA op = decodeInstruction(bits);
    std::stringstream ss;
    switch(op) {
        default: ss << "UNKNOWN[" << getISAOpName(op) << "]"; break;
#define R_TYPE(name, opcode, funct7, funct3, execution, precedence)            \
    case ISA::rv32i_##name:                                                    \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", x" << instruction::getRs2(bits); \
        break;
#define I_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv32i_##name:                                                    \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", "                                \
           << instruction::signext32<12>(instruction::getITypeImm(bits));      \
        break;
#define S_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv32i_##name:                                                    \
        ss << #name << " x" << instruction::getRs2(bits) << ", "               \
           << instruction::signext32<12>(instruction::getSTypeImm(bits))       \
           << "(x" << instruction::getRs1(bits) << ")";                        \
        break;
#define B_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv32i_##name:                                                    \
        ss << #name << " x" << instruction::getRs1(bits) << ", x"              \
           << instruction::getRs2(bits) << ", "                                \
           << "0x" << std::hex                                                 \
           << (instruction::signext64<12>(instruction::getBTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define U_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv32i_##name:                                                    \
        ss << #name << " x" << instruction::getRd(bits) << ", "                \
           << (instruction::getUTypeImm(bits) >> 12);                          \
        break;
#define J_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv32i_##name:                                                    \
        ss << #name << " x" << (instruction::getRd(bits)) << ", "              \
           << "0x" << std::hex                                                 \
           << (instruction::signext32<20>(instruction::getJTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    case ISA::rv32i_##name: ss << "custom_" #name; break;
#include "isa/rv32i.inc"
#define R_TYPE(name, opcode, funct7, funct3, execution, precedence)            \
    case ISA::rv64i_##name:                                                    \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", x" << instruction::getRs2(bits); \
        break;
#define I_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv64i_##name:                                                    \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", "                                \
           << instruction::signext64<12>(instruction::getITypeImm(bits));      \
        break;
#define S_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv64i_##name:                                                    \
        ss << #name << " x" << instruction::getRs2(bits) << ", "               \
           << instruction::signext64<12>(instruction::getSTypeImm(bits))       \
           << "(x" << instruction::getRs1(bits) << ")";                        \
        break;
#define B_TYPE(name, opcode, funct3, execution, precedence)                    \
    case ISA::rv64i_##name:                                                    \
        ss << #name << " x" << instruction::getRs1(bits) << ", x"              \
           << instruction::getRs2(bits) << ", "                                \
           << "0x" << std::hex                                                 \
           << (instruction::signext64<12>(instruction::getBTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define U_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv64i_##name:                                                    \
        ss << #name << " x" + << instruction::getRd(bits) << ", "              \
           << (instruction::getUTypeImm(bits) >> 12);                          \
        break;
#define J_TYPE(name, opcode, execution, precedence)                            \
    case ISA::rv64i_##name:                                                    \
        ss << #name << " x" + << instruction::getRd(bits) << ", "              \
           << "0x" << std::hex                                                 \
           << (instruction::signext32<20>(instruction::getJTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define CUSTOM(name, opcode, matcher, execution, precedence)                   \
    case ISA::rv64i_##name: ss << "custom_" #name; break;
#include "isa/rv64i.inc"
    }
    return ss.str();
}

} // namespace isa

class Hart64 {
  private:
    Hart64State hs;

    TraceMode trace_mode;
    Trace trace_inst;

  public:
    Hart64(mem::MemoryImage& m, TraceMode tm = TraceMode::NONE)
        : hs(m, tm), trace_mode(tm), trace_inst("INSTRUCTION TRACE") {
        trace_inst.setState((trace_mode & TraceMode::INSTRUCTION));
    }

    void execute(uint64_t start_address) {
        hs.pc = start_address;
        while(1) {
            try {
                auto inst = hs.getInstWord();
                trace_inst << "PC[" << Trace::doubleword << hs.pc << "] = ";
                trace_inst << Trace::word << inst;
                trace_inst << "; " << isa::disassemble(inst, hs.pc);
                trace_inst << std::endl;
                isa::executeInstruction(inst, hs);
            } catch(const std::exception& e) {
                std::cerr << e.what() << std::endl;
                break;
            }
        }
    }
};

} // namespace cpu

#endif
