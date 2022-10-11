#include "isa-begin.inc"

I_TYPE(lwu,
       0b0000011,
       0b110,
       uint64_t address = IMM_I_TYPE_SEXT64 + hs.GPR_RF[RS1];
       hs.GPR_RF[RD] = hs.memimg.word(address);
       hs.pc += 4;
       , 0)
I_TYPE(ld,
       0b0000011,
       0b011,
       uint64_t address = IMM_I_TYPE_SEXT64 + hs.GPR_RF[RS1];
       hs.GPR_RF[RD] = hs.memimg.doubleword(address);
       hs.pc += 4;
       , 0)
S_TYPE(sd,
       0b0100011,
       0b011,
       uint64_t address = IMM_S_TYPE_SEXT64 + hs.GPR_RF[RS1];
       hs.memimg.doubleword(address) = hs.GPR_RF[RS2] & 0xFFFFFFFFFFFFFFFF;
       hs.pc += 4;
       , 0)
CUSTOM(slli,
       0b0010011,
       return instruction::getOpcode(bits) == 0b0010011 &&
              instruction::getFunct3(bits) == 0b001 &&
              instruction::getBitsFromMSB<31 COMMA 6>(bits) == 0b000000;
       , hs.GPR_RF[RD] = hs.GPR_RF[RS1] << SHAMT6;
       hs.pc += 4;
       , 0)
CUSTOM(srli,
       0b0010011,
       return instruction::getOpcode(bits) == 0b0010011 &&
              instruction::getFunct3(bits) == 0b101 &&
              instruction::getBitsFromMSB<31 COMMA 6>(bits) == 0b000000;
       , hs.GPR_RF[RD] = hs.GPR_RF[RS1] >> SHAMT6;
       hs.pc += 4;
       , 0)
CUSTOM(srai,
       0b0010011,
       return instruction::getOpcode(bits) == 0b0010011 &&
              instruction::getFunct3(bits) == 0b101 &&
              instruction::getBitsFromMSB<31 COMMA 6>(bits) == 0b010000;
       , hs.GPR_RF[RD] = hs.GPR_RF[RS1] >> SHAMT6;
       hs.pc += 4;
       , 0)
I_TYPE(addiw,
       0b0011011,
       0b000,
       hs.GPR_RF[RD] =
           SIGNEXT64(IMM_I_TYPE_SEXT32 + SIGNEXT32(hs.GPR_RF[RS1], 32), 32);
       hs.pc += 4;
       , 0)
R_TYPE(slliw,
       0b0011011,
       0b001,
       0b0000000,
       hs.GPR_RF[RD] = SIGNEXT32(hs.GPR_RF[RS1], 32) << SHAMT5;
       hs.pc += 4;
       , 0)
R_TYPE(srliw,
       0b0011011,
       0b101,
       0b0100000,
       hs.GPR_RF[RD] = SIGNEXT32(hs.GPR_RF[RS1], 32) >> SHAMT5;
       hs.pc += 4;
       , 0)
R_TYPE(sraiw,
       0b0011011,
       0b101,
       0b0100000,
       hs.GPR_RF[RD] = SIGNEXT32(hs.GPR_RF[RS1], 32) >> SHAMT5;
       hs.pc += 4;
       , 0)
R_TYPE(addw,
       0b0111011,
       0b000,
       0b0000000,
       hs.GPR_RF[RD] = SIGNEXT64(
           SIGNEXT32(hs.GPR_RF[RS1], 32) + SIGNEXT32(hs.GPR_RF[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(subw,
       0b0111011,
       0b000,
       0b0100000,
       hs.GPR_RF[RD] = SIGNEXT64(
           SIGNEXT32(hs.GPR_RF[RS1], 32) - SIGNEXT32(hs.GPR_RF[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(sllw,
       0b0111011,
       0b001,
       0b0000000,
       hs.GPR_RF[RD] = SIGNEXT64(
           SIGNEXT32(hs.GPR_RF[RS1], 32) << SIGNEXT32(hs.GPR_RF[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(srlw,
       0b0111011,
       0b101,
       0b0000000,
       hs.GPR_RF[RD] = SIGNEXT64(
           SIGNEXT32(hs.GPR_RF[RS1], 32) >> SIGNEXT32(hs.GPR_RF[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(sraw,
       0b0111011,
       0b101,
       0b0100000,
       hs.GPR_RF[RD] = SIGNEXT64(
           SIGNEXT32(hs.GPR_RF[RS1], 32) >> SIGNEXT32(hs.GPR_RF[RS2], 32),
           32);
       hs.pc += 4;
       , 0)

/*overload sll,srl,sra from RV32I so they are defined with 6 bits*/
R_TYPE(sll,
       0b0110011,
       0b0000000,
       0b001,
       hs.GPR_RF[RD] = hs.GPR_RF[RS1] << (hs.GPR_RF[RS2] & 0x3F);
       , 0)
R_TYPE(srl,
       0b0110011,
       0b0000000,
       0b101,
       hs.GPR_RF[RD] = hs.GPR_RF[RS1] >> (hs.GPR_RF[RS2] & 0x3F);
       hs.pc += 4;
       , 0)
R_TYPE(sra,
       0b0110011,
       0b0100000,
       0b101,
       hs.GPR_RF[RD] = hs.GPR_RF[RS1] >> (hs.GPR_RF[RS2] & 0x3F);
       hs.pc += 4;
       , 0)

#include "isa-end.inc"
