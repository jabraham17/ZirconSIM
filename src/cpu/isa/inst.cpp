#include "inst.h"

#include <sstream>

#include "instruction_match.h"

namespace isa {

namespace inst {

namespace internal {
extern const std::string& getOpcodeNameFromTable(Opcode op);
extern const std::string& getOpcodeNiceNameFromTable(Opcode op);
extern Opcode decodeInstruction(uint32_t bits);
extern void executeInstruction(uint32_t bits, cpu::HartState& hs);
extern std::string disassemble(uint32_t bits, uint32_t pc, bool color);

extern std::string colorReset(bool doColor);
extern std::string colorError(bool doColor);
extern std::string colorOpcode(bool doColor);
extern std::string colorReg(bool doColor);
extern std::string colorNumber(bool doColor);

} // namespace internal

const std::string& Opcode::getName(Opcode op) {
    switch(op) {
        default: break;
    }
    return internal::getOpcodeNameFromTable(op);
}
const std::string& Opcode::getNiceName(Opcode op) {
    switch(op) {
        default: break;
    }
    return internal::getOpcodeNiceNameFromTable(op);
}

Opcode decodeInstruction(uint32_t bits) {
    Opcode op = internal::decodeInstruction(bits);
    if(op != Opcode::UNKNOWN) return op;
    // custom logic
    return op;
}
std::string fields(uint32_t bits) {
    // print func separated as R, I, S, B, U, J
    std::stringstream ss;

    ss << "R-Type |f7     |rs2  |rs1  |f3 |rd   |opcode \n";
    ss << "       |" << std::bitset<7>(instruction::getFunct7(bits)) << "|"
       << std::bitset<5>(instruction::getRs2(bits)) << "|"
       << std::bitset<5>(instruction::getRs1(bits)) << "|"
       << std::bitset<3>(instruction::getFunct3(bits)) << "|"
       << std::bitset<5>(instruction::getRd(bits)) << "|"
       << std::bitset<7>(instruction::getOpcode(bits));
    ss << "\n";
    ss << "I-Type |i11:0        |rs1  |f3 |rd   |opcode \n";
    ss << "       |"
       << std::bitset<12>(instruction::getBitsFromLSB<20, 12>(bits)) << " |"
       << std::bitset<5>(instruction::getRs1(bits)) << "|"
       << std::bitset<3>(instruction::getFunct3(bits)) << "|"
       << std::bitset<5>(instruction::getRd(bits)) << "|"
       << std::bitset<7>(instruction::getOpcode(bits));
    ss << "\n";
    ss << "S-Type |i11:5  |rs2  |rs1  |f3 |i4:0 |opcode \n";
    ss << "       |" << std::bitset<7>(instruction::getBitsFromLSB<25, 7>(bits))
       << "|" << std::bitset<5>(instruction::getRs2(bits)) << "|"
       << std::bitset<5>(instruction::getRs1(bits)) << "|"
       << std::bitset<3>(instruction::getFunct3(bits)) << "|"
       << std::bitset<5>(instruction::getBitsFromLSB<7, 5>(bits)) << "|"
       << std::bitset<7>(instruction::getOpcode(bits));
    ss << "\n";
    return ss.str();
}
void executeInstruction(uint32_t bits, cpu::HartState& hs) {
    Opcode op = decodeInstruction(bits);
    switch(op) {
        default: break;
    }
    internal::executeInstruction(bits, hs);
}

std::string disassemble(uint32_t bits, uint32_t pc, bool color) {
    Opcode op = decodeInstruction(bits);
    switch(op) {
        default: break;
        case Opcode::rv32i_auipc: {
            std::stringstream ss;
            ss << internal::colorOpcode(color) << Opcode::getNiceName(op)
               << internal::colorReset(color) << " "
               << internal::colorReg(color) << "x" << instruction::getRd(bits)
               << internal::colorReset(color) << ", "
               << internal::colorNumber(color) << "0x" << std::hex
               << (instruction::getUTypeImm(bits) >> 12)
               << internal::colorReset(color);
            return ss.str();
        }
        case Opcode::rv32i_lb:
        case Opcode::rv32i_lh:
        case Opcode::rv32i_lw:
        case Opcode::rv32i_lbu:
        case Opcode::rv32i_lhu:
        case Opcode::rv64i_lwu:
        case Opcode::rv64i_ld: {
            std::stringstream ss;
            ss << internal::colorOpcode(color) << Opcode::getNiceName(op)
               << internal::colorReset(color) << " "
               << internal::colorReg(color) << "x" << instruction::getRd(bits)
               << internal::colorReset(color) << ", "
               << internal::colorNumber(color)
               << instruction::signext64<12>(instruction::getITypeImm(bits))
               << internal::colorReset(color) << "("
               << internal::colorReg(color) << "x" << instruction::getRs1(bits)
               << internal::colorReset(color) << ")";
            return ss.str();
        }
    }
    return internal::disassemble(bits, pc, color);
}

}; // namespace inst
}; // namespace isa
