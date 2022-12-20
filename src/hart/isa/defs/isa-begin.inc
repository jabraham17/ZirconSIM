#ifndef R_TYPE
#define R_TYPE(prefix, name, opcode, funct7, funct3, execution, precedence)
#endif

#ifndef I_TYPE
#define I_TYPE(prefix, name, opcode, funct3, execution, precedence)
#endif

#ifndef S_TYPE
#define S_TYPE(prefix, name, opcode, funct3, execution, precedence)
#endif

#ifndef B_TYPE
#define B_TYPE(prefix, name, opcode, funct3, execution, precedence)
#endif

#ifndef U_TYPE
#define U_TYPE(prefix, name, opcode, execution, precedence)
#endif

#ifndef J_TYPE
#define J_TYPE(prefix, name, opcode, execution, precedence)
#endif

#ifndef CUSTOM
#define CUSTOM(prefix, name, opcode, matcher, printer, execution, precedence)
#endif

#ifndef ZIRCON_CPU_ISA_COMMON_BEGIN_HELPER_
#define ZIRCON_CPU_ISA_COMMON_BEGIN_HELPER_

#define OPCODE (instruction::getOpcode(bits))
#define FUNCT7 (instruction::getFunct7(bits))
#define FUNCT3 (instruction::getFunct3(bits))
#define RD (instruction::getRd(bits))
#define RS2 (instruction::getRs2(bits))
#define RS1 (instruction::getRs1(bits))
#define SHAMT5 (instruction::getShamt5(bits))
#define SHAMT6 (instruction::getShamt6(bits))
#define IMM_I_TYPE (instruction::getITypeImm(bits))
#define IMM_S_TYPE (instruction::getSTypeImm(bits))
#define IMM_B_TYPE (instruction::getBTypeImm(bits))
#define IMM_U_TYPE (instruction::getUTypeImm(bits))
#define IMM_J_TYPE (instruction::getJTypeImm(bits))
#define SIGNEXT128(x, B) (instruction::signext128<B>(x))
#define SIGNEXT64(x, B) (instruction::signext64<B>(x))
#define SIGNEXT32(x, B) (instruction::signext32<B>(x))
#define IMM_I_TYPE_SEXT64                                                      \
    (instruction::signext64<12>(instruction::getITypeImm(bits)))
#define IMM_S_TYPE_SEXT64                                                      \
    (instruction::signext64<12>(instruction::getSTypeImm(bits)))
#define IMM_B_TYPE_SEXT64                                                      \
    (instruction::signext64<13>(instruction::getBTypeImm(bits)))
#define IMM_U_TYPE_SEXT64                                                      \
    (instruction::signext64<32>(instruction::getUTypeImm(bits)))
#define IMM_J_TYPE_SEXT64                                                      \
    (instruction::signext64<20>(instruction::getJTypeImm(bits)))
#define IMM_I_TYPE_SEXT32                                                      \
    (instruction::signext32<12>(instruction::getITypeImm(bits)))
#define IMM_S_TYPE_SEXT32                                                      \
    (instruction::signext32<12>(instruction::getSTypeImm(bits)))
#define IMM_B_TYPE_SEXT32                                                      \
    (instruction::signext32<13>(instruction::getBTypeImm(bits)))
#define IMM_U_TYPE_SEXT32                                                      \
    (instruction::signext32<32>(instruction::getUTypeImm(bits)))
#define IMM_J_TYPE_SEXT32                                                      \
    (instruction::signext32<20>(instruction::getJTypeImm(bits)))

#define INSTRUCTION_WIDTH (opcode.getInstructionSize())
#define NEXT_INSTRUCTION                                                       \
    do {                                                                       \
        hs().pc += INSTRUCTION_WIDTH;                                          \
    } while(0)

#define COMMA ,
#endif
