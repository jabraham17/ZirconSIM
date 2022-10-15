#include "isa-begin.inc"

R_TYPE(rv32m,
       mul,
       0b0110011,
       0b0000001,
       0b000,
       hs.rf.GPR[RD] = hs.rf.GPR[RS1] * hs.rf.GPR[RS2];
       hs.pc += 4;
       , 0)
R_TYPE(rv32m,
       mulh,
       0b0110011,
       0b0000001,
       0b001,
       hs.rf.GPR[RD] =
           ((SIGNEXT128(hs.rf.GPR[RS1], 64) * SIGNEXT128(hs.rf.GPR[RS2], 64)) >>
            64);
       hs.pc += 4;
       , 0)
R_TYPE(rv32m,
       mulhsu,
       0b0110011,
       0b0000001,
       0b010,
       hs.rf.GPR[RD] =
           ((__int128_t(hs.rf.GPR[RS1]) * __int128_t(hs.rf.GPR[RS2])) >> 64);
       hs.pc += 4;
       , 0)
R_TYPE(rv32m,
       mulhu,
       0b0110011,
       0b0000001,
       0b011,
       hs.rf.GPR[RD] =
           ((SIGNEXT128(hs.rf.GPR[RS1], 64) * __int128_t(hs.rf.GPR[RS2])) >>
            64);
       hs.pc += 4;
       , 0)
R_TYPE(rv32m,
       div,
       0b0110011,
       0b0000001,
       0b100,
       hs.rf.GPR[RD] = SIGNEXT64(hs.rf.GPR[RS1], 64) /
                       SIGNEXT64(hs.rf.GPR[RS2], 64);
       hs.pc += 4;
       , 0)
R_TYPE(rv32m,
       divu,
       0b0110011,
       0b0000001,
       0b101,
       hs.rf.GPR[RD] = hs.rf.GPR[RS1] / hs.rf.GPR[RS2];
       hs.pc += 4;
       , 0)
R_TYPE(rv32m,
       rem,
       0b0110011,
       0b0000001,
       0b110,
       hs.rf.GPR[RD] = SIGNEXT64(hs.rf.GPR[RS1], 64) %
                       SIGNEXT64(hs.rf.GPR[RS2], 64);
       hs.pc += 4;
       , 0)
R_TYPE(rv32m,
       remu,
       0b0110011,
       0b0000001,
       0b111,
       hs.rf.GPR[RD] = hs.rf.GPR[RS1] % hs.rf.GPR[RS2];
       hs.pc += 4;
       , 0)
