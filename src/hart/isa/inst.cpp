#include "inst.h"

#include "hart/hart.h"

#include <sstream>

#include "instruction_match.h"

namespace isa {

namespace inst {

namespace internal {
extern const std::string& getOpcodeNameFromTable(Opcode op);
extern const std::string& getOpcodeNiceNameFromTable(Opcode op);
extern const std::string& getPrefixFromTable(Opcode op);
extern const Opcode getOpcodeFromTable(std::string prefix, std::string name);
extern const Opcode getOpcodeFromTable(std::string name);

extern uint64_t getOpcodeFieldFromTable(Opcode op);
extern uint64_t getFunct7FieldFromTable(Opcode op);
extern uint64_t getFunct3FieldFromTable(Opcode op);

extern Opcode decodeInstruction(uint32_t bits);
extern void executeInstruction(uint32_t bits, hart::HartState& hs);
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
const std::string& Opcode::getBareName(Opcode op) {
    switch(op) {
        default: break;
    }
    return internal::getOpcodeNiceNameFromTable(op);
}
const std::string& Opcode::getPrefix(Opcode op) {
    switch(op) {
        default: break;
    }
    return internal::getPrefixFromTable(op);
}

const Opcode Opcode::lookupName(std::string prefix, std::string name) {
    return internal::getOpcodeFromTable(prefix, name);
}
const Opcode Opcode::lookupName(std::string name) {
    return internal::getOpcodeFromTable(name);
}

uint64_t Opcode::getOpcodeField(Opcode op) {
    return internal::getOpcodeFieldFromTable(op);
}
uint64_t Opcode::getFunct7Field(Opcode op) {
    return internal::getFunct7FieldFromTable(op);
}
uint64_t Opcode::getFunct3Field(Opcode op) {
    return internal::getFunct3FieldFromTable(op);
}

Opcode decodeInstruction(uint32_t bits) {
    Opcode op = internal::decodeInstruction(bits);
    if(op != Opcode::UNKNOWN) return op;
    // custom logic
    return op;
}

}; // namespace inst
}; // namespace isa
