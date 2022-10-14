#include "inst.h"
#include "instruction_match.h"
#include <sstream>

namespace isa {

namespace inst {

namespace internal {
const std::string& getOpcodeNameFromTable(Opcode op);
extern Opcode decodeInstruction(uint32_t bits);
extern void executeInstruction(uint32_t bits, cpu::HartState& hs);
extern std::string disassemble(uint32_t bits, uint32_t pc);

} // namespace internal

const std::string& Opcode::getName(Opcode op) {
    switch(op) {
        default: break;
    }
    return internal::getOpcodeNameFromTable(op);
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

std::string disassemble(uint32_t bits, uint32_t pc) {
    Opcode op = decodeInstruction(bits);
    switch(op) {
        default: break;
    }
    return internal::disassemble(bits, pc);
}

}; // namespace inst
}; // namespace isa
