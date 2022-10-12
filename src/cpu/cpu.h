
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

    Hart64State(mem::MemoryImage& m, TraceMode tm = TraceMode::NONE)
        : memimg(m) {
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
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
#define R_TYPE(prefix, name, ...) prefix##_##name,
#define I_TYPE(prefix, name, ...) prefix##_##name,
#define S_TYPE(prefix, name, ...) prefix##_##name,
#define B_TYPE(prefix, name, ...) prefix##_##name,
#define U_TYPE(prefix, name, ...) prefix##_##name,
#define J_TYPE(prefix, name, ...) prefix##_##name,
#define CUSTOM(prefix, name, ...) prefix##_##name,
#include "isa/defs/instructions.inc"
    LAST_OP
};

std::string
    ISA_NAME_TABLE[uint64_t(ISA::LAST_OP) - uint64_t(ISA::FIRST_OP) + 1] = {
        "FIRST_OP",
        "UNKNOWN",
#define R_TYPE(prefix, name, ...) #prefix "_" #name,
#define I_TYPE(prefix, name, ...) #prefix "_" #name,
#define S_TYPE(prefix, name, ...) #prefix "_" #name,
#define B_TYPE(prefix, name, ...) #prefix "_" #name,
#define U_TYPE(prefix, name, ...) #prefix "_" #name,
#define J_TYPE(prefix, name, ...) #prefix "_" #name,
#define CUSTOM(prefix, name, ...) #prefix "_" #name,
#include "isa/defs/instructions.inc"
        "LAST_OP"};

std::string getISAOpName(ISA op) { return ISA_NAME_TABLE[uint64_t(op)]; }

uint64_t ISA_PRECEDENCE[uint64_t(ISA::LAST_OP) - uint64_t(ISA::FIRST_OP) + 1] =
    {0,
     0,
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    precedence,
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence) precedence,
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence) precedence,
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence) precedence,
#define U_TYPE(prefix, name, opcode, execution, precedence) precedence,
#define J_TYPE(prefix, name, opcode, execution, precedence) precedence,
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence) precedence,
#include "isa/defs/instructions.inc"
     0};

#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
    bool prefix##_##name##_matcher_func([[maybe_unused]] uint32_t bits) {      \
        do {                                                                   \
            matcher;                                                           \
        } while(0);                                                            \
    }
#include "isa/defs/instructions.inc"

uint64_t getISAPrecedence(ISA op) { return ISA_PRECEDENCE[uint64_t(op)]; }

ISA decodeInstruction(uint32_t bits) {
    ISA matched = ISA::UNKNOWN;
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct7(bits) == funct7 &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::prefix##_##name;                                        \
    }
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::prefix##_##name;                                        \
    }
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::prefix##_##name;                                        \
    }
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    if(instruction::getOpcode(bits) == opcode &&                               \
       instruction::getFunct3(bits) == funct3 &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::prefix##_##name;                                        \
    }
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::prefix##_##name;                                        \
    }
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    if(instruction::getOpcode(bits) == opcode &&                               \
       (matched == ISA::UNKNOWN || precedence < getISAPrecedence(matched))) {  \
        matched = ISA::prefix##_##name;                                        \
    }
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
    if(prefix##_##name##_matcher_func(bits)) matched = ISA::prefix##_##name;
#include "isa/defs/instructions.inc"

    return matched;
}

void executeInstruction(uint32_t bits, Hart64State& hs) {
    ISA op = decodeInstruction(bits);
    switch(op) {
        default: throw IllegalInstructionException();

#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    case ISA::prefix##_##name: do { execution;                                 \
        } while(0);                                                            \
        break;
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case ISA::prefix##_##name: do { execution;                                 \
        } while(0);                                                            \
        break;
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case ISA::prefix##_##name: do { execution;                                 \
        } while(0);                                                            \
        break;
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case ISA::prefix##_##name: do { execution;                                 \
        } while(0);                                                            \
        break;
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    case ISA::prefix##_##name: do { execution;                                 \
        } while(0);                                                            \
        break;
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    case ISA::prefix##_##name: do { execution;                                 \
        } while(0);                                                            \
        break;
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
    case ISA::prefix##_##name: do { execution;                                 \
        } while(0);                                                            \
        break;
#include "isa/defs/instructions.inc"
    }
}

std::string disassemble(uint32_t bits, uint32_t pc = 0) {
    ISA op = decodeInstruction(bits);
    std::stringstream ss;
    switch(op) {
        default: ss << "UNKNOWN[" << getISAOpName(op) << "]"; break;
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)    \
    case ISA::prefix##_##name:                                                 \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", x" << instruction::getRs2(bits); \
        break;
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case ISA::prefix##_##name:                                                 \
        ss << #name << " x" << instruction::getRd(bits) << ", x"               \
           << instruction::getRs1(bits) << ", "                                \
           << instruction::signext64<12>(instruction::getITypeImm(bits));      \
        break;
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case ISA::prefix##_##name:                                                 \
        ss << #name << " x" << instruction::getRs2(bits) << ", "               \
           << instruction::signext64<12>(instruction::getSTypeImm(bits))       \
           << "(x" << instruction::getRs1(bits) << ")";                        \
        break;
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)            \
    case ISA::prefix##_##name:                                                 \
        ss << #name << " x" << instruction::getRs1(bits) << ", x"              \
           << instruction::getRs2(bits) << ", "                                \
           << "0x" << std::hex                                                 \
           << (instruction::signext64<12>(instruction::getBTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define U_TYPE(prefix, name, opcode, execution, precedence)                    \
    case ISA::prefix##_##name:                                                 \
        ss << #name << " x" << instruction::getRd(bits) << ", "                \
           << (instruction::getUTypeImm(bits) >> 12);                          \
        break;
#define J_TYPE(prefix, name, opcode, execution, precedence)                    \
    case ISA::prefix##_##name:                                                 \
        ss << #name << " x" << instruction::getRd(bits) << ", "                \
           << "0x" << std::hex                                                 \
           << (instruction::signext32<20>(instruction::getJTypeImm(bits)) +    \
               pc);                                                            \
        break;
#define CUSTOM(prefix, name, opcode, matcher, execution, precedence)           \
    case ISA::prefix##_##name: ss << "custom_" #name; break;
#include "isa/defs/instructions.inc"
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

    void init() {
        // allocate a stack region at 0x7fffffff00000000-0x7fffffff00010000
        hs.GPR_RF[2] = 0x7fffffff00010000;
        hs.memimg.allocate(0x7fffffff00000000, 0x10008);
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
                if(trace_mode)
                    std::cout << std::setfill('-') << std::setw(80) << "\n";
            } catch(const std::exception& e) {
                std::cerr << e.what() << std::endl;
                break;
            }
        }
    }
};

} // namespace cpu

#endif
