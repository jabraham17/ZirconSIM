// clang-format off

#include "isa-begin.inc"

/*out of tree instruction take precedence over ones defined here*/
/*this allows an application to override existing ones*/
#include "generated-definition/instructions.inc"

#include "rv32i.inc"
#include "rv64i.inc"
#include "rv32m.inc"
#include "rv64m.inc"
#include "rv32a.inc"

#include "isa-end.inc"

// clang-format on
