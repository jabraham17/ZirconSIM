
// #include "instruction.h"

// #include <iomanip>
// #include <iostream>

// // std::array<const char[], 2> chars= {"lui", "auipc"};

// namespace Instructions {

// #define INST_STR(inst) inst_name_##inst##_
// #define INST_ACTION(inst) inst_action_##inst##_
// #define MAKE_TEMPS(inst, ACTION) \
//     const char INST_STR(inst)[] = #inst; \ void
//     inst_action_##inst##_(Hart64State& hs) { ACTION }

// // rv32i
// MAKE_TEMPS(lui, auto I = hs.getInstruction();
// printf("here %d\n", I.getRd());
//            hs.rf.reg(I.getRd()).setValue(I.getImm());
//            )
// MAKE_TEMPS(auipc, )
// MAKE_TEMPS(jal, )
// MAKE_TEMPS(jalr, )
// MAKE_TEMPS(beq, )
// MAKE_TEMPS(bne, )
// MAKE_TEMPS(blt, )
// MAKE_TEMPS(bge, )
// MAKE_TEMPS(bltu, )
// MAKE_TEMPS(bgte, )
// MAKE_TEMPS(lb, )
// MAKE_TEMPS(lh, )
// MAKE_TEMPS(lw, )
// MAKE_TEMPS(lbu, )
// MAKE_TEMPS(lhu, )
// MAKE_TEMPS(sb, )
// MAKE_TEMPS(sh, )
// MAKE_TEMPS(sw, )
// MAKE_TEMPS(addi, )
// MAKE_TEMPS(slti, )
// MAKE_TEMPS(sltiu, )
// MAKE_TEMPS(xori, )
// MAKE_TEMPS(ori, )
// MAKE_TEMPS(andi, )
// MAKE_TEMPS(slli, )
// MAKE_TEMPS(srli, )
// MAKE_TEMPS(srai, )
// MAKE_TEMPS(add, )
// MAKE_TEMPS(sub, )
// MAKE_TEMPS(sll, )
// MAKE_TEMPS(slt, )
// MAKE_TEMPS(sltu, )
// MAKE_TEMPS(xor, )
// MAKE_TEMPS(srl, )
// MAKE_TEMPS(sra, )
// MAKE_TEMPS(or, )
// MAKE_TEMPS(and, )
// MAKE_TEMPS(fence, )
// MAKE_TEMPS(ecall, )
// MAKE_TEMPS(ebreak, )

// // rv64i
// MAKE_TEMPS(lwu, )
// MAKE_TEMPS(ld, )
// MAKE_TEMPS(sd, )
// // MAKE_TEMPS(slli, )
// // MAKE_TEMPS(srli, )
// // MAKE_TEMPS(srai, )
// MAKE_TEMPS(addiw, )
// MAKE_TEMPS(slliw, )
// MAKE_TEMPS(srliw, )
// MAKE_TEMPS(sraiw, )
// MAKE_TEMPS(addw, )
// MAKE_TEMPS(subw, )
// MAKE_TEMPS(sllw, )
// MAKE_TEMPS(srlw, )
// MAKE_TEMPS(sraw, )

// #undef MAKE_TEMPS

// const Instruction* isa_32i[] = {
//       new UType<INST_STR(lui), INST_ACTION(lui), 0b0110111>(),
//       new UType<INST_STR(auipc), INST_ACTION(auipc), 0b0010111>(),
//       new JType<INST_STR(jal), INST_ACTION(jal), 0b1101111>(),
//       new IType<INST_STR(jalr), INST_ACTION(jalr), 0b1100111, 0b000>(),
//       new BType<INST_STR(beq), INST_ACTION(beq), 0b1100011, 0b000>(),
//       new BType<INST_STR(bne), INST_ACTION(bne), 0b1100011, 0b001>(),
//       new BType<INST_STR(blt), INST_ACTION(blt), 0b1100011, 0b100>(),
//       new BType<INST_STR(bge), INST_ACTION(bge), 0b1100011, 0b101>(),
//       new BType<INST_STR(bltu), INST_ACTION(bltu), 0b1100011, 0b110>(),
//       new BType<INST_STR(bgte), INST_ACTION(bgte), 0b1100011, 0b111>(),
//       new IType<INST_STR(lb), INST_ACTION(lb), 0b0000011, 0b000>(),
//       new IType<INST_STR(lh), INST_ACTION(lh), 0b0000011, 0b001>(),
//       new IType<INST_STR(lw), INST_ACTION(lw), 0b0000011, 0b010>(),
//       new IType<INST_STR(lbu), INST_ACTION(lbu), 0b0000011, 0b100>(),
//       new IType<INST_STR(lhu), INST_ACTION(lhu), 0b0000011, 0b101>(),
//       new SType<INST_STR(sb), INST_ACTION(sb), 0b0100011, 0b000>(),
//       new SType<INST_STR(sh), INST_ACTION(sh), 0b0100011, 0b001>(),
//       new SType<INST_STR(sw), INST_ACTION(sw), 0b0100011, 0b010>(),
//       new IType<INST_STR(addi), INST_ACTION(addi), 0b0010011, 0b000>(),
//       new IType<INST_STR(slti), INST_ACTION(slti), 0b0010011, 0b000>(),
//       new IType<INST_STR(sltiu), INST_ACTION(sltiu), 0b0010011, 0b010>(),
//       new IType<INST_STR(xori), INST_ACTION(xori), 0b0010011, 0b100>(),
//       new IType<INST_STR(ori), INST_ACTION(ori), 0b0010011, 0b110>(),
//       new IType<INST_STR(andi), INST_ACTION(andi), 0b0010011, 0b111>(),
//       new RType<
//           INST_STR(slli),
//           INST_ACTION(slli),
//           0b0010011,
//           0b001,
//           0b0000000>(),
//       new RType<
//           INST_STR(srli),
//           INST_ACTION(srli),
//           0b0010011,
//           0b101,
//           0b0000000>(),
//       new RType<
//           INST_STR(srai),
//           INST_ACTION(srai),
//           0b0010011,
//           0b101,
//           0b0100000>(),
//       new RType<INST_STR(add), INST_ACTION(add), 0b0110011, 0b0000000,
//       0b000>(), new RType<INST_STR(sub), INST_ACTION(sub), 0b0110011,
//       0b0100000, 0b000>(), new RType<INST_STR(sll), INST_ACTION(sll),
//       0b0110011, 0b0000000, 0b001>(), new RType<INST_STR(slt),
//       INST_ACTION(slt), 0b0110011, 0b0000000, 0b010>(), new RType<
//           INST_STR(sltu),
//           INST_ACTION(sltu),
//           0b0110011,
//           0b0000000,
//           0b011>(),
//       new RType<INST_STR(xor), INST_ACTION(xor), 0b0110011, 0b0000000,
//       0b100>(), new RType<INST_STR(srl), INST_ACTION(srl), 0b0110011,
//       0b0000000, 0b101>(), new RType<INST_STR(sra), INST_ACTION(sra),
//       0b0110011, 0b0100000, 0b101>(), new RType<INST_STR(or),
//       INST_ACTION(or), 0b0110011, 0b0000000, 0b110>(), new
//       RType<INST_STR(and), INST_ACTION(and), 0b0110011, 0b0000000, 0b111>(),
//       // new Type<INST_STR(fence),INST_ACTION(fence), 0b>(),
//       // new Type<INST_STR(ecall),INST_ACTION(ecall), 0b>(),
//       // new Type<INST_STR(ebreak),INST_ACTION(ebreak), 0b>(),
//       nullptr};

// const Instruction* isa_64i[] = {
//     new IType<INST_STR(lwu), INST_ACTION(lwu), 0b0000011, 0b110>(),
//     new IType<INST_STR(ld), INST_ACTION(ld), 0b0000011, 0b011>(),
//     new SType<INST_STR(sd), INST_ACTION(sd), 0b0100011, 0b011>(),
//     // new IType<INST_STR(slli),INST_ACTION(slli), 0b0010011, 0b001>(),
//     //fixme
//     // new IType<INST_STR(srli),INST_ACTION(srli), 0b0010011, 0b101>(),
//     //fixme
//     // new IType<INST_STR(srai),INST_ACTION(srai), 0b0010011, 0b101>(),
//     //fixme new IType<INST_STR(addiw), INST_ACTION(addiw), 0b0011011,
//     0b000>(), new RType<
//         INST_STR(slliw),
//         INST_ACTION(slliw),
//         0b0011011,
//         0b001,
//         0b0000000>(),
//     new RType<
//         INST_STR(srliw),
//         INST_ACTION(srliw),
//         0b0011011,
//         0b101,
//         0b0100000>(),
//     new RType<
//         INST_STR(sraiw),
//         INST_ACTION(sraiw),
//         0b0011011,
//         0b101,
//         0b0100000>(),
//     new RType<INST_STR(addw), INST_ACTION(addw), 0b0111011, 0b000,
//     0b0000000>(), new RType<INST_STR(subw), INST_ACTION(subw), 0b0111011,
//     0b000, 0b0100000>(), new RType<INST_STR(sllw), INST_ACTION(sllw),
//     0b0111011, 0b001, 0b0000000>(), new RType<INST_STR(srlw),
//     INST_ACTION(srlw), 0b0111011, 0b101, 0b0000000>(), new
//     RType<INST_STR(sraw), INST_ACTION(sraw), 0b0111011, 0b101, 0b0100000>(),
//     nullptr};

// const Instruction* getInstruction(uint32_t bits) {
//     const Instruction temp(bits);
//     for(unsigned idx = 0; isa_32i[idx] != nullptr; idx++) {
//         const Instruction* inst = isa_32i[idx];
//         // inst must go first so we get the correct ==
//         if(*inst == temp) return inst;
//     }
//     for(unsigned idx = 0; isa_64i[idx] != nullptr; idx++) {
//         const Instruction* inst = isa_64i[idx];
//         // inst must go first so we get the correct ==
//         if(*inst == temp) return inst;
//     }
//     return nullptr;
// }

// } // namespace Instructions
