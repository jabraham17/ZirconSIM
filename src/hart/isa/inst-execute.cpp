#include "inst-execute.h"

#include "hart/hart.h"

#include <sstream>

#include "instruction_match.h"

namespace isa {
namespace inst {

namespace internal {
extern void executeInstruction(uint32_t bits, hart::HartState& hs);
extern std::string disassemble(uint32_t bits, uint32_t pc, bool color);

extern std::string colorReset(bool doColor);
extern std::string colorError(bool doColor);
extern std::string colorOpcode(bool doColor);
extern std::string colorReg(bool doColor);
extern std::string colorNumber(bool doColor);

} // namespace internal

void executeInstruction(uint32_t bits, hart::HartState& hs) {
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
} // namespace inst
} // namespace isa
