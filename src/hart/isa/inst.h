#ifndef ZIRCON_HART_ISA_INST_H_
#define ZIRCON_HART_ISA_INST_H_

#include "hart/hart.h"

#include <string>

namespace isa {

namespace inst {

struct Opcode {
    using ValueType = uint64_t;
    ValueType _value;
    enum Opcodes {
        UNKNOWN,
#define R_TYPE(prefix, name, ...) prefix##_##name,
#define I_TYPE(prefix, name, ...) prefix##_##name,
#define S_TYPE(prefix, name, ...) prefix##_##name,
#define B_TYPE(prefix, name, ...) prefix##_##name,
#define U_TYPE(prefix, name, ...) prefix##_##name,
#define J_TYPE(prefix, name, ...) prefix##_##name,
#define CUSTOM(prefix, name, ...) prefix##_##name,
#include "defs/instructions.inc"
    };

    Opcode(ValueType v) : _value(v) {}
    Opcode() : _value(UNKNOWN) {}

    operator ValueType() const { return this->_value; }
    explicit operator const std::string&() const {
        return Opcode::getName(*this);
    }

    static const std::string& getName(Opcode op);
    static const std::string& getNiceName(Opcode op);
    static constexpr size_t size();

    bool isRType() const;
    bool isIType() const;
    bool isSType() const;
    bool isBType() const;
    bool isUType() const;
    bool isJType() const;
    bool isCustomType() const;

    int getInstructionSize() const;
};

Opcode decodeInstruction(uint32_t bits);
std::string fields(uint32_t bits);
void executeInstruction(uint32_t bits, hart::HartState& hs);
std::string disassemble(uint32_t bits, uint32_t pc = 0, bool color = false);

}; // namespace inst
}; // namespace isa
#endif
