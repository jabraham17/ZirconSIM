#include "commonbegin.inc"

// clang-format off
U_TYPE(lui, 0b0110111, 
    hs.rf[RD] = IMM_U_TYPE;
    hs.pc += 4
, 0)
U_TYPE(auipc, 0b0010111,
    hs.rf[RD] = IMM_U_TYPE + hs.pc;
    hs.pc += 4
, 0)
J_TYPE(jal, 0b1101111,
    hs.rf[RD] = hs.pc + 4;
    hs.pc = IMM_J_TYPE_SEXT64 + hs.pc;
, 0)
I_TYPE(jalr, 0b1100111, 0b000,
    uint64_t new_pc = (IMM_I_TYPE_SEXT64 + hs.rf[RS1]) & (~(1)); // clear LSB
    hs.rf[RD] = hs.pc + 4;
    hs.pc = new_pc;
, 0)
B_TYPE(beq, 0b1100011, 0b000, 
    uint64_t target = IMM_B_TYPE_SEXT64 + hs.pc;
    if(hs.rf[RS1] == hs.rf[RS2]) {
        hs.pc = target;
    }
    else {
        hs.pc += 4;
    }
, 0)
B_TYPE(bne, 0b1100011, 0b001,
    uint64_t target = IMM_B_TYPE_SEXT64 + hs.pc;
    if(hs.rf[RS1] != hs.rf[RS2]) {
        hs.pc = target;
    }
    else {
        hs.pc += 4;
    }
, 0)
B_TYPE(blt, 0b1100011, 0b100,
    uint64_t target = IMM_B_TYPE_SEXT64 + hs.pc;
    if(int64_t(hs.rf[RS1]) < int64_t(hs.rf[RS2])) {
        hs.pc = target;
    }
    else {
        hs.pc += 4;
    }
, 0)
B_TYPE(bge, 0b1100011, 0b101,
    uint64_t target = IMM_B_TYPE_SEXT64 + hs.pc;
    if(int64_t(hs.rf[RS1]) >= int64_t(hs.rf[RS2])) {
        hs.pc = target;
    }
    else {
        hs.pc += 4;
    }
, 0)
B_TYPE(bltu, 0b1100011, 0b110,
    uint64_t target = IMM_B_TYPE_SEXT64 + hs.pc;
    if(uint64_t(hs.rf[RS1]) < uint64_t(hs.rf[RS2])) {
        hs.pc = target;
    }
    else {
        hs.pc += 4;
    }
, 0)
B_TYPE(bgeu, 0b1100011, 0b111,
    uint64_t target = IMM_B_TYPE_SEXT64 + hs.pc;
    if(uint64_t(hs.rf[RS1]) >= uint64_t(hs.rf[RS2])) {
        hs.pc = target;
    }
    else {
        hs.pc += 4;
    }
, 0)
I_TYPE(lb, 0b0000011, 0b000,
    uint64_t address = IMM_I_TYPE_SEXT64 + hs.rf[RS1];
    hs.rf[RD] = SIGNEXT64(hs.mem.byte(address), 8);
    hs.pc += 4;
, 0)
I_TYPE(lh, 0b0000011, 0b001,
    uint64_t address = IMM_I_TYPE_SEXT64 + hs.rf[RS1];
    hs.rf[RD] = SIGNEXT64(hs.mem.halfword(address), 16);
    hs.pc += 4;
, 0)
I_TYPE(lw, 0b0000011, 0b010,
    uint64_t address = IMM_I_TYPE_SEXT64 + hs.rf[RS1];
    hs.rf[RD] = SIGNEXT64(hs.mem.word(address), 32);
    hs.pc += 4;
, 0)
I_TYPE(lbu, 0b0000011, 0b100,
    uint64_t address = IMM_I_TYPE_SEXT64 + hs.rf[RS1];
    hs.rf[RD] = hs.mem.byte(address);
    hs.pc += 4;
, 0)
I_TYPE(lhu, 0b0000011, 0b101,
    uint64_t address = IMM_I_TYPE_SEXT64 + hs.rf[RS1];
    hs.rf[RD] = hs.mem.halfword(address);
    hs.pc += 4;
, 0)
S_TYPE(sb, 0b0100011, 0b000,
    uint64_t address = IMM_S_TYPE_SEXT64 + hs.rf[RS1];
    hs.mem.byte(address) = hs.rf[RS2] & 0xFF;
    hs.pc += 4;
, 0)
S_TYPE(sh, 0b0100011, 0b001,
    uint64_t address = IMM_S_TYPE_SEXT64 + hs.rf[RS1];
    hs.mem.halfword(address) = hs.rf[RS2] & 0xFFFF;
    hs.pc += 4;
, 0)
S_TYPE(sw, 0b0100011, 0b010,
    uint64_t address = IMM_S_TYPE_SEXT64 + hs.rf[RS1];
    hs.mem.word(address) = hs.rf[RS2] & 0xFFFFFFFF;
    hs.pc += 4;
, 0)
I_TYPE(addi, 0b0010011, 0b000,
    hs.rf[RD] = IMM_I_TYPE_SEXT64 + hs.rf[RS1];
    hs.pc += 4;
, 0)
I_TYPE(slti, 0b0010011, 0b000,
    std::cout << "UNIMPLEMENTED\n";
    hs.pc += 4;
, 0)
I_TYPE(sltiu, 0b0010011, 0b010,
    std::cout << "UNIMPLEMENTED\n";
    hs.pc += 4;
, 0)
I_TYPE(xori, 0b0010011, 0b100,
    hs.rf[RD] = IMM_I_TYPE_SEXT64 ^ hs.rf[RS1];
    hs.pc += 4;
, 0)
I_TYPE(ori, 0b0010011, 0b110,
    hs.rf[RD] = IMM_I_TYPE_SEXT64 | hs.rf[RS1];
    hs.pc += 4;
, 0)
I_TYPE(andi, 0b0010011, 0b111,
    hs.rf[RD] = IMM_I_TYPE_SEXT64 & hs.rf[RS1];
    hs.pc += 4;
, 0)
R_TYPE(slli, 0b0010011, 0b001, 0b0000000,
    hs.rf[RD] = hs.rf[RS1] << SHAMT5;
    hs.pc += 4;
, 1 /*RV64I version takes precedence*/)
R_TYPE(srli, 0b0010011, 0b101, 0b0000000,
    hs.rf[RD] = hs.rf[RS1] >> SHAMT5;
    hs.pc += 4;
, 1 /*RV64I version takes precedence*/)
R_TYPE(srai, 0b0010011, 0b101, 0b0100000,
    hs.rf[RD] = hs.rf[RS1] >> SHAMT5;
    hs.pc += 4;
, 1 /*RV64I version takes precedence*/)
R_TYPE(add, 0b0110011, 0b0000000, 0b000,
    hs.rf[RD] = hs.rf[RS1] + hs.rf[RS2];
    hs.pc += 4;
, 0)
R_TYPE(sub, 0b0110011, 0b0100000, 0b000,
    hs.rf[RD] = hs.rf[RS2] - hs.rf[RS1];
    hs.pc += 4;
, 0)
R_TYPE(sll, 0b0110011, 0b0000000, 0b001,
    hs.rf[RD] = hs.rf[RS1] << (hs.rf[RS2] & 0x1F);
, 1 /*RV64I version takes precedence*/)
R_TYPE(slt, 0b0110011, 0b0000000, 0b010,
    std::cout << "UNIMPLEMENTED\n";
    hs.pc += 4;
, 0)
R_TYPE(sltu, 0b0110011, 0b0000000, 0b011,
    std::cout << "UNIMPLEMENTED\n";
    hs.pc += 4;
, 0)
R_TYPE(xor, 0b0110011, 0b0000000, 0b100,
    hs.rf[RD] = hs.rf[RS1] ^ hs.rf[RS2];
    hs.pc += 4;
, 0)
R_TYPE(srl, 0b0110011, 0b0000000, 0b101,
    hs.rf[RD] = hs.rf[RS1] >> (hs.rf[RS2] & 0x1F);
    hs.pc += 4;
, 1 /*RV64I version takes precedence*/)
R_TYPE(sra, 0b0110011, 0b0100000, 0b101,
    hs.rf[RD] = hs.rf[RS1] >> (hs.rf[RS2] & 0x1F);
    hs.pc += 4;
, 1 /*RV64I version takes precedence*/)
R_TYPE(or, 0b0110011, 0b0000000, 0b110,
    hs.rf[RD] = hs.rf[RS1] | hs.rf[RS2];
    hs.pc += 4;
, 0)
R_TYPE(and, 0b0110011, 0b0000000, 0b111,
    hs.rf[RD] = hs.rf[RS1] & hs.rf[RS2];
    hs.pc += 4;
, 0)
CUSTOM(fence, 0b0001111,
    return 
        instruction::getOpcode(bits) == 0b0001111 && 
        instruction::getFunct3(bits) == 0b000;
,
    std::cout << "UNIMPLEMENTED\n";
    hs.pc += 4;
, 0)
CUSTOM(ecall, 0b1110011,
    return 
        instruction::getOpcode(bits) == 0b1110011 && 
        instruction::getBitRange<31 COMMA 7>(bits) == 0;
,
    std::cout << "UNIMPLEMENTED\n";
    hs.pc += 4;
, 0)
CUSTOM(ebreak, 0b1110011,
    return 
        instruction::getOpcode(bits) == 0b1110011 && 
        instruction::getBitRange<19 COMMA 7>(bits) == 0b0 &&
        instruction::getBitRange<31 COMMA 21>(bits) == 0b0 &&
        instruction::getBit<20>(bits) == 0b1;

,
    std::cout << "UNIMPLEMENTED\n";
    hs.pc += 4;
, 0)
//clang-format on

#include "commonend.inc"
