
#ifndef RISCV_SIM_INSTRUCTION_H_
#define RISCV_SIM_INSTRUCTION_H_

// #include <array>
// #include <cstdint>
// #include <ostream>
// #include <iostream>
// #include <string>
// #include "cpu.h"
// #include <functional>

// namespace Instructions {

// class Instruction {

//   public:
//     Instruction() : bits_(0) {}
//     Instruction(uint32_t bits) : bits_(bits), name_("UNKNOWN") {}
//     Instruction(uint32_t bits, std::string name) : bits_(bits), name_(name)
//     {} virtual ~Instruction() {}

//     uint32_t& bits() { return bits_; }
//     const uint32_t& bits() const { return bits_; }
//     std::string& name() { return name_; }
//     const std::string& name() const { return name_; }

//     virtual uint32_t getOpcode() { return bits() & 0x3F; }
//     virtual uint32_t getOpcode() const { return bits() & 0x3F; }
//     virtual uint32_t getFunct7() { return (bits() >> 25) & 0x7F; }
//     virtual uint32_t getFunct7() const { return (bits() >> 25) & 0x7F; }
//     virtual uint32_t getRs2() {
//         std::cout << "hello\n" << std::hex << bits() << "\n";
//         return (bits() >> 20) & 0x1F;
//     }
//         virtual uint32_t getRs2() const {
//         std::cout << "hello\n" << std::hex << bits() << "\n";
//         return (bits() >> 20) & 0x1F;
//     }
//     virtual uint32_t getRs1() { return (bits() >> 15) & 0x1F; }
//     virtual uint32_t getRs1() const { return (bits() >> 15) & 0x1F; }
//     virtual uint32_t getFunct3() { return (bits() >> 12) & 0x7; }
//     virtual uint32_t getFunct3() const { return (bits() >> 12) & 0x7; }
//     virtual uint32_t getRd() { return (bits() >> 7) & 0x1F; }
//     virtual uint32_t getRd() const { return (bits() >> 7) & 0x1F; }
//     virtual uint32_t getImm() { return bits(); }
//     virtual uint32_t getImm() const { return bits(); }

//     virtual bool operator==(const Instruction& other) {
//         return this->getOpcode() == other.getOpcode();
//     }
//     virtual bool operator==(const Instruction& other) const {
//         return this->getOpcode() == other.getOpcode();
//     }

//     virtual void print(std::ostream& os) { os << this->name(); }
//     virtual void print(std::ostream& os) const { os << this->name(); }

//     virtual void execute(Hart64State& hs) {

//     }

//   protected:
//     uint32_t bits_;
//     std::string name_;
// };

// using InstructionAction = void(*)(Hart64State&);

// template <const char* inst_name, InstructionAction IA, uint32_t opcode>
// class BaseOpcode : public Instruction {

//   public:
//     BaseOpcode() {
//         this->bits() |= opcode & 0x3F;
//         this->name() = std::string(inst_name);
//     }
//     virtual void execute(Hart64State& hs) {
//         printf("here\n");
//         IA(hs);
//     }
// };

// template <const char* name, InstructionAction IA, uint32_t opcode, uint32_t
// funct7, uint32_t funct3> class RType : public BaseOpcode<name, IA, opcode> {
//   public:
//     RType() {
//         this->bits() |= (funct7 & 0x7F) << 25;
//         this->bits() |= (funct3 & 0x7) << 12;
//     }
//     virtual bool operator==(const Instruction& other) override {

//         return this->getOpcode() == other.getOpcode() &&
//                this->getFunct7() == other.getFunct7() &&
//                this->getFunct3() == other.getFunct3();
//     }

//     virtual void print(std::ostream& os) override {
//         os << this->name() << " " << this->getRd() << ", " << this->getRs1()
//            << ", " << this->getRs2();
//     }    virtual void print(std::ostream& os) const override {
//         os << this->name() << " " << this->getRd() << ", " << this->getRs1()
//            << ", " << this->getRs2();
//     }
// };

// template <const char* name, InstructionAction IA, uint32_t opcode, uint32_t
// funct3> class IType : public BaseOpcode<name, IA, opcode> {
//   public:
//     IType() {
//         this->bits() |= (funct3 & 0x7) << 12;
//     }

//     virtual uint32_t getImm() override { return (this->bits() >> 20) & 0xFFF;
//     }

//     virtual bool operator==(const Instruction& other) override {
//         return this->getOpcode() == other.getOpcode() &&
//                this->getFunct3() == other.getFunct3();
//     }
// };

// template <const char* name, InstructionAction IA, uint32_t opcode, uint32_t
// funct3> class SType : public BaseOpcode<name, IA, opcode> {
//   public:
//     SType() { this->bits() |= (funct3 & 0x7) << 12; }
//     virtual uint32_t getImm() override {
//         uint32_t imm11_5 = (this->bits() >> 25) & 0x7F;
//         uint32_t imm4_0 = (this->bits() >> 7) & 0x1F;
//         return (imm11_5 << 5) | (imm4_0);
//     }

//     virtual bool operator==(const Instruction& other) override {
//         return this->getOpcode() == other.getOpcode() &&
//                this->getFunct3() == other.getFunct3();
//     }
// };

// template <const char* name, InstructionAction IA, uint32_t opcode, uint32_t
// funct3> class BType : public BaseOpcode<name, IA, opcode> {
//   public:
//     BType() { this->bits() |= (funct3 & 0x7) << 12; }
//     virtual uint32_t getImm() override {
//         uint32_t imm12 = (this->bits() >> 31) & 0x1;
//         uint32_t imm10_5 = (this->bits() >> 25) & 0x3F;
//         uint32_t imm4_1 = (this->bits() >> 8) & 0xF;
//         uint32_t imm11 = (this->bits() >> 7) & 0x1;
//         return (imm12 << 12) | (imm10_5 << 5) | (imm4_1 << 1) | (imm11 <<
//         11);
//     }

//     virtual bool operator==(const Instruction& other) override {
//         return this->getOpcode() == other.getOpcode() &&
//                this->getFunct3() == other.getFunct3();
//     }
// };

// template <const char* name, InstructionAction IA, uint32_t opcode>
// class UType : public BaseOpcode<name, IA, opcode> {
//   public:
//     virtual uint32_t getImm() override {
//         uint32_t imm31_12 = (this->bits() >> 12) & 0xFFFFF;
//         return imm31_12 << 12;
//     }
// };

// template <const char* name, InstructionAction IA, uint32_t opcode>
// class JType : public BaseOpcode<name, IA, opcode> {
//   public:
//     virtual uint32_t getImm() override {
//         uint32_t imm20 = (this->bits() >> 31) & 0x1;
//         uint32_t imm10_1 = (this->bits() >> 30) & 0x3FF;
//         uint32_t imm11 = (this->bits() >> 20) & 0x1;
//         uint32_t imm19_12 = (this->bits() >> 12) & 0xFF;
//         return (imm20 << 20) | (imm10_1 << 1) | (imm11 << 11) |
//                (imm19_12 << 12);
//     }
// };

// extern const Instruction* isa_32i[];
// const Instruction* getInstruction(uint32_t bits);

// } // namespace Instructions

#endif
