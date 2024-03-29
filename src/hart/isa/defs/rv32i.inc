U_TYPE(rv32i, lui, 0b0110111, hs().rf().GPR[RD] = IMM_U_TYPE_SEXT64;
       NEXT_INSTRUCTION, 0)
U_TYPE(rv32i, auipc, 0b0010111, hs().rf().GPR[RD] = IMM_U_TYPE_SEXT64 + hs().pc;
       NEXT_INSTRUCTION, 0)
J_TYPE(rv32i, jal, 0b1101111, hs().rf().GPR[RD] = hs().pc + INSTRUCTION_WIDTH;
       hs().pc = IMM_J_TYPE_SEXT64 + hs().pc;
       , 0)
I_TYPE(rv32i,
       jalr,
       0b1100111,
       0b000,
       types::Address new_pc = (IMM_I_TYPE_SEXT64 + hs().rf().GPR[RS1]) &
                               (~(1)); // clear LSB
       hs().rf().GPR[RD] = hs().pc + INSTRUCTION_WIDTH;
       hs().pc = new_pc;
       , 0)
B_TYPE(
    rv32i,
    beq,
    0b1100011,
    0b000,
    types::Address target = IMM_B_TYPE_SEXT64 + hs().pc;
    if(hs().rf().GPR[RS1] == hs().rf().GPR[RS2]) { hs().pc = target; } else {
        NEXT_INSTRUCTION;
    },
    0)
B_TYPE(
    rv32i,
    bne,
    0b1100011,
    0b001,
    types::Address target = IMM_B_TYPE_SEXT64 + hs().pc;
    if(hs().rf().GPR[RS1] != hs().rf().GPR[RS2]) { hs().pc = target; } else {
        NEXT_INSTRUCTION;
    },
    0)
B_TYPE(
    rv32i,
    blt,
    0b1100011,
    0b100,
    types::Address target = IMM_B_TYPE_SEXT64 + hs().pc;
    if(types::SignedInteger(hs().rf().GPR[RS1]) <
       types::SignedInteger(hs().rf().GPR[RS2])) { hs().pc = target; } else {
        NEXT_INSTRUCTION;
    },
    0)
B_TYPE(
    rv32i,
    bge,
    0b1100011,
    0b101,
    types::Address target = IMM_B_TYPE_SEXT64 + hs().pc;
    if(types::SignedInteger(hs().rf().GPR[RS1]) >=
       types::SignedInteger(hs().rf().GPR[RS2])) { hs().pc = target; } else {
        NEXT_INSTRUCTION;
    },
    0)
B_TYPE(
    rv32i,
    bltu,
    0b1100011,
    0b110,
    types::Address target = IMM_B_TYPE_SEXT64 + hs().pc;
    if(types::UnsignedInteger(hs().rf().GPR[RS1]) <
       types::UnsignedInteger(hs().rf().GPR[RS2])) { hs().pc = target; } else {
        NEXT_INSTRUCTION;
    },
    0)
B_TYPE(
    rv32i,
    bgeu,
    0b1100011,
    0b111,
    types::Address target = IMM_B_TYPE_SEXT64 + hs().pc;
    if(types::UnsignedInteger(hs().rf().GPR[RS1]) >=
       types::UnsignedInteger(hs().rf().GPR[RS2])) { hs().pc = target; } else {
        NEXT_INSTRUCTION;
    },
    0)
I_TYPE(rv32i,
       lb,
       0b0000011,
       0b000,
       types::Address address = IMM_I_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().rf().GPR[RD] = SIGNEXT64(hs().mem().byte(address), 8);
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       lh,
       0b0000011,
       0b001,
       types::Address address = IMM_I_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().rf().GPR[RD] = SIGNEXT64(hs().mem().halfword(address), 16);
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       lw,
       0b0000011,
       0b010,
       types::Address address = IMM_I_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().rf().GPR[RD] = SIGNEXT64(hs().mem().word(address), 32);
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       lbu,
       0b0000011,
       0b100,
       types::Address address = IMM_I_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().rf().GPR[RD] = hs().mem().byte(address);
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       lhu,
       0b0000011,
       0b101,
       types::Address address = IMM_I_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().rf().GPR[RD] = hs().mem().halfword(address);
       NEXT_INSTRUCTION;
       , 0)
S_TYPE(rv32i,
       sb,
       0b0100011,
       0b000,
       types::Address address = IMM_S_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().mem().byte(address) = hs().rf().GPR[RS2] & 0xFF;
       NEXT_INSTRUCTION;
       , 0)
S_TYPE(rv32i,
       sh,
       0b0100011,
       0b001,
       types::Address address = IMM_S_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().mem().halfword(address) = hs().rf().GPR[RS2] & 0xFFFF;
       NEXT_INSTRUCTION;
       , 0)
S_TYPE(rv32i,
       sw,
       0b0100011,
       0b010,
       types::Address address = IMM_S_TYPE_SEXT64 + hs().rf().GPR[RS1];
       hs().mem().word(address) = hs().rf().GPR[RS2] & 0xFFFFFFFF;
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       addi,
       0b0010011,
       0b000,
       hs().rf().GPR[RD] = IMM_I_TYPE_SEXT64 + hs().rf().GPR[RS1];
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       slti,
       0b0010011,
       0b010,
       hs().rf().GPR[RD] = SIGNEXT64(hs().rf().GPR[RS1], 64) < IMM_I_TYPE_SEXT64
                               ? 1
                               : 0;
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       sltiu,
       0b0010011,
       0b011,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] <
                                   types::UnsignedInteger(IMM_I_TYPE_SEXT64)
                               ? 1
                               : 0;
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       xori,
       0b0010011,
       0b100,
       hs().rf().GPR[RD] = IMM_I_TYPE_SEXT64 ^ hs().rf().GPR[RS1];
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       ori,
       0b0010011,
       0b110,
       hs().rf().GPR[RD] = IMM_I_TYPE_SEXT64 | hs().rf().GPR[RS1];
       NEXT_INSTRUCTION;
       , 0)
I_TYPE(rv32i,
       andi,
       0b0010011,
       0b111,
       hs().rf().GPR[RD] = IMM_I_TYPE_SEXT64 & hs().rf().GPR[RS1];
       NEXT_INSTRUCTION;
       , 0)
R_TYPE(rv32i,
       slli,
       0b0010011,
       0b0000000,
       0b001,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] << SHAMT5;
       NEXT_INSTRUCTION;
       , 1 /*RV64I version takes precedence*/)
R_TYPE(rv32i,
       srli,
       0b0010011,
       0b0000000,
       0b101,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] >> SHAMT5;
       NEXT_INSTRUCTION;
       , 1 /*RV64I version takes precedence*/)
R_TYPE(rv32i,
       srai,
       0b0010011,
       0b0100000,
       0b101,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] >> SHAMT5;
       NEXT_INSTRUCTION;
       , 1 /*RV64I version takes precedence*/)
R_TYPE(rv32i,
       add,
       0b0110011,
       0b0000000,
       0b000,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] + hs().rf().GPR[RS2];
       NEXT_INSTRUCTION;
       , 0)
R_TYPE(rv32i,
       sub,
       0b0110011,
       0b0100000,
       0b000,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] - hs().rf().GPR[RS2];
       NEXT_INSTRUCTION;
       , 0)
R_TYPE(rv32i,
       sll,
       0b0110011,
       0b0000000,
       0b001,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] << (hs().rf().GPR[RS2] & 0x1F);
       NEXT_INSTRUCTION;
       , 1 /*RV64I version takes precedence*/)
R_TYPE(rv32i,
       slt,
       0b0110011,
       0b0000000,
       0b010,
       hs().rf().GPR[RD] = SIGNEXT64(hs().rf().GPR[RS1], 64) <
                                   SIGNEXT64(hs().rf().GPR[RS2], 64)
                               ? 1
                               : 0;
       NEXT_INSTRUCTION;
       , 0)
R_TYPE(rv32i,
       sltu,
       0b0110011,
       0b0000000,
       0b011,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] < hs().rf().GPR[RS2] ? 1 : 0;
       NEXT_INSTRUCTION;
       , 0)
R_TYPE(rv32i,
       xor,
       0b0110011,
       0b0000000,
       0b100,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] ^ hs().rf().GPR[RS2];
       NEXT_INSTRUCTION;
       , 0)
R_TYPE(rv32i,
       srl,
       0b0110011,
       0b0000000,
       0b101,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] >> (hs().rf().GPR[RS2] & 0x1F);
       NEXT_INSTRUCTION;
       , 1 /*RV64I version takes precedence*/)
R_TYPE(rv32i,
       sra,
       0b0110011,
       0b0100000,
       0b101,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] >> (hs().rf().GPR[RS2] & 0x1F);
       NEXT_INSTRUCTION;
       , 1 /*RV64I version takes precedence*/)
R_TYPE(rv32i,
       or
       ,
       0b0110011,
       0b0000000,
       0b110,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] | hs().rf().GPR[RS2];
       NEXT_INSTRUCTION;
       , 0)
R_TYPE(rv32i,
       and,
       0b0110011,
       0b0000000,
       0b111,
       hs().rf().GPR[RD] = hs().rf().GPR[RS1] & hs().rf().GPR[RS2];
       NEXT_INSTRUCTION;
       , 0)
CUSTOM(rv32i,
       fence,
       0b0001111,
       return instruction::getOpcode(bits) == 0b0001111 &&
              instruction::getFunct3(bits) == 0b000;
       ,
       return internal::colorOpcode(color) + "fence" +
              internal::colorReset(color);
       , NEXT_INSTRUCTION;
       , 0)
CUSTOM(rv32i,
       ecall,
       0b1110011,
       return instruction::getOpcode(bits) == 0b1110011 &&
              instruction::getBitRange<31 COMMA 7>(bits) == 0;
       ,
       return internal::colorOpcode(color) + "ecall" +
              internal::colorReset(color);
       , sys::emulate(hs);
       NEXT_INSTRUCTION;
       , 0)
CUSTOM(rv32i,
       ebreak,
       0b1110011,
       return instruction::getOpcode(bits) == 0b1110011 &&
              instruction::getBitRange<19 COMMA 7>(bits) == 0b0 &&
              instruction::getBitRange<31 COMMA 21>(bits) == 0b0 &&
              instruction::getBit<20>(bits) == 0b1;
       ,
       return internal::colorOpcode(color) + "ebreak" +
              internal::colorReset(color);
       , hs().pause();
       NEXT_INSTRUCTION;
       , 0)
