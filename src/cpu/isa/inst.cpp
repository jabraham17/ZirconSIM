#include "inst.h"

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
