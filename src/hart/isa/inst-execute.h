#ifndef ZIRCON_HART_ISA_INST_EXECUTE_H_
#define ZIRCON_HART_ISA_INST_EXECUTE_H_

#include "inst.h"

#include "hart/hart.h"

#include <string>

namespace isa {

namespace inst {

void executeInstruction(uint32_t bits, hart::HartState& hs);
std::string disassemble(uint32_t bits, uint32_t pc = 0, bool color = false);

}; // namespace inst
}; // namespace isa
#endif
