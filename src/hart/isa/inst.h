#ifndef ZIRCON_HART_ISA_INST_H_
#define ZIRCON_HART_ISA_INST_H_

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

    static const std::string& getPrefix(Opcode op);
    static const std::string& getBareName(Opcode op);

    static const Opcode lookupName(std::string prefix, std::string name);
    static const Opcode lookupName(std::string name);

    static uint64_t getOpcodeField(Opcode op);
    static uint64_t getFunct7Field(Opcode op);
    static uint64_t getFunct3Field(Opcode op);

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

}; // namespace inst
}; // namespace isa
#endif
