#include "isa-begin.inc"

R_TYPE(rv64m,
       mulw,
       0b0111011,
       0b0000001,
       0b000,
       hs().rf().GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs().rf().GPR[RS1], 32) * SIGNEXT32(hs().rf().GPR[RS2], 32),
           32);
       hs().pc += 4;
       , 0)
R_TYPE(rv64m,
       divw,
       0b0111011,
       0b0000001,
       0b100,
       hs().rf().GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs().rf().GPR[RS1], 32) / SIGNEXT32(hs().rf().GPR[RS2], 32),
           32);
       hs().pc += 4;
       , 0)
R_TYPE(rv64m,
       divuw,
       0b0111011,
       0b0000001,
       0b101,
       hs().rf().GPR[RD] = SIGNEXT64(
           (hs().rf().GPR[RS1] & instruction::getMask<32>()) /
               (hs().rf().GPR[RS2] & instruction::getMask<32>()),
           32);
       hs().pc += 4;
       , 0)
R_TYPE(rv64m,
       remw,
       0b0111011,
       0b0000001,
       0b110,
       hs().rf().GPR[RD] = SIGNEXT64(
           SIGNEXT32(hs().rf().GPR[RS1], 32) % SIGNEXT32(hs().rf().GPR[RS2], 32),
           32);
       hs().pc += 4;
       , 0)
R_TYPE(rv64m,
       remuw,
       0b0111011,
       0b0000001,
       0b111,
       hs().rf().GPR[RD] = SIGNEXT64(
           (hs().rf().GPR[RS1] & instruction::getMask<32>()) %
               (hs().rf().GPR[RS2] & instruction::getMask<32>()),
           32);
       hs().pc += 4;
       , 0)
