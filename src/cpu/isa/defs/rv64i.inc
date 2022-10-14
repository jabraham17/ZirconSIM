#include "isa-begin.inc"

I_TYPE(rv64i,
       lwu,
       0b0000011,
       0b110,
       uint64_t address = IMM_I_TYPE_SEXT64 + hs.rf.GPR[RS1];
       hs.rf.GPR[RD] = hs.memimg.word(address);
       hs.pc += 4;
       , 0)
I_TYPE(rv64i,
       ld,
       0b0000011,
       0b011,
       uint64_t address = IMM_I_TYPE_SEXT64 + hs.rf.GPR[RS1];
       hs.rf.GPR[RD] = hs.memimg.doubleword(address);
       hs.pc += 4;
       , 0)
S_TYPE(rv64i,
       sd,
       0b0100011,
       0b011,
       uint64_t address = IMM_S_TYPE_SEXT64 + hs.rf.GPR[RS1];
       hs.memimg.doubleword(address) = hs.rf.GPR[RS2] & 0xFFFFFFFFFFFFFFFF;
       hs.pc += 4;
       , 0)
CUSTOM(rv64i,
       slli,
       0b0010011,
       return instruction::getOpcode(bits) == 0b0010011 &&
              instruction::getFunct3(bits) == 0b001 &&
              instruction::getBitsFromMSB<31 COMMA 6>(bits) == 0b000000;
       , hs.rf.GPR[RD] = hs.rf.GPR[RS1] << SHAMT6;
       hs.pc += 4;
       , 0)
CUSTOM(rv64i,
       srli,
       0b0010011,
       return instruction::getOpcode(bits) == 0b0010011 &&
              instruction::getFunct3(bits) == 0b101 &&
              instruction::getBitsFromMSB<31 COMMA 6>(bits) == 0b000000;
       , hs.rf.GPR[RD] = hs.rf.GPR[RS1] >> SHAMT6;
       hs.pc += 4;
       , 0)
CUSTOM(rv64i,
       srai,
       0b0010011,
       return instruction::getOpcode(bits) == 0b0010011 &&
              instruction::getFunct3(bits) == 0b101 &&
              instruction::getBitsFromMSB<31 COMMA 6>(bits) == 0b010000;
       , hs.rf.GPR[RD] = hs.rf.GPR[RS1] >> SHAMT6;
       hs.pc += 4;
       , 0)
I_TYPE(rv64i,
       addiw,
       0b0011011,
       0b000,
       hs.rf.GPR[RD] =
           SIGNEXT64(IMM_I_TYPE_SEXT32 + SIGNEXT32(hs.rf.GPR[RS1], 32), 32);
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       slliw,
       0b0011011,
       0b001,
       0b0000000,
       hs.rf.GPR[RD] = SIGNEXT32(hs.rf.GPR[RS1], 32) << SHAMT5;
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       srliw,
       0b0011011,
       0b101,
       0b0100000,
       hs.rf.GPR[RD] = SIGNEXT32(hs.rf.GPR[RS1], 32) >> SHAMT5;
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       sraiw,
       0b0011011,
       0b101,
       0b0100000,
       hs.rf.GPR[RD] = SIGNEXT32(hs.rf.GPR[RS1], 32) >> SHAMT5;
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       addw,
       0b0111011,
       0b000,
       0b0000000,
       hs.rf.GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs.rf.GPR[RS1], 32) + SIGNEXT32(hs.rf.GPR[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       subw,
       0b0111011,
       0b000,
       0b0100000,
       hs.rf.GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs.rf.GPR[RS1], 32) - SIGNEXT32(hs.rf.GPR[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       sllw,
       0b0111011,
       0b001,
       0b0000000,
       hs.rf.GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs.rf.GPR[RS1], 32) << SIGNEXT32(hs.rf.GPR[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       srlw,
       0b0111011,
       0b101,
       0b0000000,
       hs.rf.GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs.rf.GPR[RS1], 32) >> SIGNEXT32(hs.rf.GPR[RS2], 32),
           32);
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       sraw,
       0b0111011,
       0b101,
       0b0100000,
       hs.rf.GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs.rf.GPR[RS1], 32) >> SIGNEXT32(hs.rf.GPR[RS2], 32),
           32);
       hs.pc += 4;
       , 0)

/*overload sll,srl,sra from RV32I so they are defined with 6 bits*/
R_TYPE(rv64i,
       sll,
       0b0110011,
       0b0000000,
       0b001,
       hs.rf.GPR[RD] = hs.rf.GPR[RS1] << (hs.rf.GPR[RS2] & 0x3F);
       , 0)
R_TYPE(rv64i,
       srl,
       0b0110011,
       0b0000000,
       0b101,
       hs.rf.GPR[RD] = hs.rf.GPR[RS1] >> (hs.rf.GPR[RS2] & 0x3F);
       hs.pc += 4;
       , 0)
R_TYPE(rv64i,
       sra,
       0b0110011,
       0b0100000,
       0b101,
       hs.rf.GPR[RD] = hs.rf.GPR[RS1] >> (hs.rf.GPR[RS2] & 0x3F);
       hs.pc += 4;
       , 0)
