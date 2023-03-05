

#include "common/argparse.hpp"
#include "common/format.h"
#include "hart/isa/inst.h"
#include "hart/isa/instruction_match.h"

#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>

/*
everything read as hex
./ib  -f I -o op -3 0x1 -7 -1 -rd 0 -rs1 -rs2 -i imm
./ib  -prefix -name
./ib  -prefix -name -rd -rs1 -rs2
*/

// TODO: does not really support custom inst
// TODO; does not support SHAMT

enum class InstructionFormat { NONE, R, I, S, B, U, J };
InstructionFormat parseInstructionFormat(const std::string& s) {
    if(s == "R" || s == "r") return InstructionFormat::R;
    if(s == "I" || s == "i") return InstructionFormat::I;
    if(s == "S" || s == "s") return InstructionFormat::S;
    if(s == "B" || s == "b") return InstructionFormat::B;
    if(s == "U" || s == "u") return InstructionFormat::U;
    if(s == "J" || s == "j") return InstructionFormat::J;
    return InstructionFormat::NONE;
}
std::ostream& operator<<(std::ostream& os, InstructionFormat i) {
    if(i == InstructionFormat::R) os << "R";
    else if(i == InstructionFormat::I) os << "I";
    else if(i == InstructionFormat::S) os << "S";
    else if(i == InstructionFormat::B) os << "B";
    else if(i == InstructionFormat::U) os << "U";
    else if(i == InstructionFormat::J) os << "J";
    else os << "NONE";
    return os;
}
InstructionFormat getFormatFromOpcode(isa::inst::Opcode op) {
    if(op.isRType()) return InstructionFormat::R;
    if(op.isIType()) return InstructionFormat::I;
    if(op.isSType()) return InstructionFormat::S;
    if(op.isBType()) return InstructionFormat::B;
    if(op.isUType()) return InstructionFormat::U;
    if(op.isJType()) return InstructionFormat::J;
    return InstructionFormat::NONE;
}

struct Instruction {
    std::string prefix;
    std::string name;
    InstructionFormat format;
    uint32_t opcode;
    uint32_t funct7;
    uint32_t funct3;
    uint32_t rd;
    uint32_t rs2;
    uint32_t rs1;
    uint32_t raw_imm;

    Instruction()
        : prefix(std::string("")), name(std::string("")),
          format(InstructionFormat::NONE), opcode(), funct7(), funct3(), rd(),
          rs2(), rs1(), raw_imm(){};

  private:
    void setInfoFromOpcode(isa::inst::Opcode op) {
        this->prefix = isa::inst::Opcode::getPrefix(op);
        this->name = isa::inst::Opcode::getBareName(op);
        this->format = getFormatFromOpcode(op);
        this->opcode = isa::inst::Opcode::getOpcodeField(op);
        this->funct7 = isa::inst::Opcode::getFunct7Field(op);
        this->funct3 = isa::inst::Opcode::getFunct3Field(op);
    }

  public:
    void setKnownInstruction(std::string prefix, std::string name) {
        auto op = isa::inst::Opcode::lookupName(prefix, name);
        setInfoFromOpcode(op);
    }
    void setKnownInstruction(std::string name) {
        auto op = isa::inst::Opcode::lookupName(name);
        setInfoFromOpcode(op);
    }

    uint32_t build() {
        switch(format) {
            case InstructionFormat::R: {
                uint32_t bits = 0;
                bits = instruction::setOpcode(bits, opcode);
                bits = instruction::setFunct7(bits, funct7);
                bits = instruction::setFunct3(bits, funct3);
                bits = instruction::setRd(bits, rd);
                bits = instruction::setRs2(bits, rs2);
                bits = instruction::setRs1(bits, rs1);
                return bits;
            }
            case InstructionFormat::I: {
                uint32_t bits = 0;
                bits = instruction::setOpcode(bits, opcode);
                bits = instruction::setFunct3(bits, funct3);
                bits = instruction::setRd(bits, rd);
                bits = instruction::setRs1(bits, rs1);
                bits = instruction::setITypeImm(bits, raw_imm);
                return bits;
            }
            case InstructionFormat::S: {
                uint32_t bits = 0;
                bits = instruction::setOpcode(bits, opcode);
                bits = instruction::setFunct3(bits, funct3);
                bits = instruction::setRs2(bits, rs2);
                bits = instruction::setRs1(bits, rs1);
                bits = instruction::setSTypeImm(bits, raw_imm);
                return bits;
            }
            case InstructionFormat::B: {
                uint32_t bits = 0;
                bits = instruction::setOpcode(bits, opcode);
                bits = instruction::setFunct3(bits, funct3);
                bits = instruction::setRs2(bits, rs2);
                bits = instruction::setRs1(bits, rs1);
                bits = instruction::setBTypeImm(bits, raw_imm);
                return bits;
            }
            case InstructionFormat::U: {
                uint32_t bits = 0;
                bits = instruction::setOpcode(bits, opcode);
                bits = instruction::setRd(bits, rd);
                bits = instruction::setUTypeImm(bits, raw_imm);
                return bits;
            }
            case InstructionFormat::J: {
                uint32_t bits = 0;
                bits = instruction::setOpcode(bits, opcode);
                bits = instruction::setRd(bits, rd);
                bits = instruction::setJTypeImm(bits, raw_imm);
                return bits;
            }
            default: {
                uint32_t bits = 0;
                // do our best, do nothing with the IMM
                if(opcode) bits = instruction::setOpcode(bits, opcode);
                if(funct7) bits = instruction::setFunct7(bits, funct7);
                if(funct3) bits = instruction::setFunct3(bits, funct3);
                if(rd) bits = instruction::setRd(bits, rd);
                if(rs2) bits = instruction::setRs2(bits, rs2);
                if(rs1) bits = instruction::setRs1(bits, rs1);
                return bits;
            }
        }
    }

    std::string info() {
        auto bits = build();
        auto op = isa::inst::decodeInstruction(bits);
        if(op != isa::inst::Opcode::UNKNOWN)
            return isa::inst::Opcode::getPrefix(op) + "." +
                   isa::inst::Opcode::getBareName(op);
        else return "UNKNOWN";
    }
};

auto get_args() {
    argparse::ArgumentParser args;

    args.add_argument("-f")
        .default_value(InstructionFormat::NONE)
        .action([](const std::string& value) {
            return parseInstructionFormat(value);
        })
        .help("which format to use");
    args.add_argument("-op")
        .default_value(std::string("0"))
        .help("opcode field, 7 bits");

    args.add_argument("-f7")
        .default_value(std::string("0"))
        .help("funct7 field, 7 bits");
    args.add_argument("-f3")
        .default_value(std::string("0"))
        .help("funct3 field, 3 bits");

    args.add_argument("-rd")
        .default_value(std::string("0"))
        .help("rd field, 5 bits");

    args.add_argument("-rs2")
        .default_value(std::string("0"))
        .help("rs2 field, 5 bits");
    args.add_argument("-rs1")
        .default_value(std::string("0"))
        .help("rs1 field, 5 bits");

    args.add_argument("-i")
        .default_value(std::string("0"))
        .help("immediate field");

    return args;
}

uint32_t parse_num(const std::string& s) {
    if(s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return std::stoull(s.substr(2), nullptr, 16);
    else if(s.size() >= 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B'))
        return std::stoull(s.substr(2), nullptr, 2);
    else return std::stoull(s, nullptr, 10);
}

Instruction get_instruction_from_args(argparse::ArgumentParser args) {
    Instruction inst;
    inst.format = args.get<InstructionFormat>("-f");
    inst.opcode = parse_num(args.get<std::string>("-op"));
    inst.funct3 = parse_num(args.get<std::string>("-f3"));
    inst.funct7 = parse_num(args.get<std::string>("-f7"));
    inst.rd = parse_num(args.get<std::string>("-rd"));
    inst.rs1 = parse_num(args.get<std::string>("-rs1"));
    inst.rs2 = parse_num(args.get<std::string>("-rs2"));
    inst.raw_imm = parse_num(args.get<std::string>("-i"));
    return inst;
}

int main(int argc, const char** argv) {

    auto args = get_args();
    try {
        args.parse_args(argc, argv);
    } catch(const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    auto inst = get_instruction_from_args(args);
    std::cout << inst.info() << ": " << common::Format::word << inst.build()
              << std::endl;

    // uint32_t x = 0xFFFF;
    // uint32_t clear = instruction::clearBitsFromLSB<8, 4>(x);
    // std::cout << "0x" << common::Format::word << x << " 0x" <<
    // common::Format::word << clear << std::endl;

    // x = 0x0000;
    // clear = instruction::setBitsFromLSB<8, 8>(x, 0xed);
    // std::cout << "0x" << common::Format::word << x << " 0x" <<
    // common::Format::word << clear << std::endl;

    return 0;
}
