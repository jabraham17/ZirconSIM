#ifndef R_TYPE
#define R_TYPE(name, opcode, funct7, funct3, execution, precedence)
#endif

#ifndef I_TYPE
#define I_TYPE(name, opcode, funct3, execution, precedence)
#endif

#ifndef S_TYPE
#define S_TYPE(name, opcode, funct3, execution, precedence)
#endif

#ifndef B_TYPE
#define B_TYPE(name, opcode, funct3, execution, precedence)
#endif

#ifndef U_TYPE
#define U_TYPE(name, opcode, execution, precedence)
#endif

#ifndef J_TYPE
#define J_TYPE(name, opcode, execution, precedence)
#endif

#ifndef CUSTOM
#define CUSTOM(name, opcode, matcher, execution, precedence)
#endif

#ifndef ZIRCON_CPU_ISA_COMMON_BEGIN_HELPER_
#define ZIRCON_CPU_ISA_COMMON_BEGIN_HELPER_

#define OPCODE (instruction::getOpcode(hs.getInstWord()))
#define FUNCT7 (instruction::getFunct7(hs.getInstWord()))
#define FUNCT3 (instruction::getFunct3(hs.getInstWord()))
#define RD (instruction::getRd(hs.getInstWord()))
#define RS2 (instruction::getRs2(hs.getInstWord()))
#define RS1 (instruction::getRs1(hs.getInstWord()))
#define SHAMT5 (instruction::getShamt5(hs.getInstWord()))
#define SHAMT6 (instruction::getShamt6(hs.getInstWord()))
#define IMM_I_TYPE (instruction::getITypeImm(hs.getInstWord()))
#define IMM_S_TYPE (instruction::getSTypeImm(hs.getInstWord()))
#define IMM_B_TYPE (instruction::getBTypeImm(hs.getInstWord()))
#define IMM_U_TYPE (instruction::getUTypeImm(hs.getInstWord()))
#define IMM_J_TYPE (instruction::getJTypeImm(hs.getInstWord()))
#define SIGNEXT64(x, B) (instruction::signext64<B>(x))
#define SIGNEXT32(x, B) (instruction::signext32<B>(x))
#define IMM_I_TYPE_SEXT64                                                      \
    (instruction::signext64<12>(instruction::getITypeImm(hs.getInstWord())))
#define IMM_S_TYPE_SEXT64                                                      \
    (instruction::signext64<12>(instruction::getSTypeImm(hs.getInstWord())))
#define IMM_B_TYPE_SEXT64                                                      \
    (instruction::signext64<12>(instruction::getBTypeImm(hs.getInstWord())))
#define IMM_U_TYPE_SEXT64                                                      \
    (instruction::signext64<20>(instruction::getUTypeImm(hs.getInstWord())))
#define IMM_J_TYPE_SEXT64                                                      \
    (instruction::signext64<20>(instruction::getJTypeImm(hs.getInstWord())))
#define IMM_I_TYPE_SEXT32                                                      \
    (instruction::signext32<12>(instruction::getITypeImm(hs.getInstWord())))
#define IMM_S_TYPE_SEXT32                                                      \
    (instruction::signext32<12>(instruction::getSTypeImm(hs.getInstWord())))
#define IMM_B_TYPE_SEXT32                                                      \
    (instruction::signext32<12>(instruction::getBTypeImm(hs.getInstWord())))
#define IMM_U_TYPE_SEXT32                                                      \
    (instruction::signext32<20>(instruction::getUTypeImm(hs.getInstWord())))
#define IMM_J_TYPE_SEXT32                                                      \
    (instruction::signext32<20>(instruction::getJTypeImm(hs.getInstWord())))
#define COMMA ,

#endif
