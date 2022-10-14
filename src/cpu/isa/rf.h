#ifndef SRC_CPU_ISA_RF_H_
#define SRC_CPU_ISA_RF_H_

#include "register.h"
#include "trace/trace.h"

namespace isa {
namespace rf {

class RegisterFile {
  public:
#define REG_CASE(...) MAKE_REGISTER(__VA_ARGS__),
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    RegisterClass<number_regs, reg_size> classname = {                         \
        #classname,                                                            \
        #reg_prefix,                                                           \
        {REGISTER_CLASS_##classname(REG_CASE)}};
#include "defs/registers.inc"

    RegisterFile(TraceMode tm = TraceMode::NONE) { setTraceMode(tm); }

    void setTraceMode(TraceMode tm) {
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    classname.setTraceMode(tm);
#include "defs/registers.inc"
    }
};
}; // namespace rf
}; // namespace isa

#endif
