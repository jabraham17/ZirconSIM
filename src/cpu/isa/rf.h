#ifndef SRC_CPU_ISA_RF_H_
#define SRC_CPU_ISA_RF_H_

#include "event/event.h"
#include "register.h"

namespace isa {
namespace rf {

enum class RegisterClassType {
#define REGISTER_CLASS(r, ...) r,
#include "defs/registers.inc"
    NONE,
};
bool isRegisterClassType(std::string s);

RegisterClassType getRegisterClassType(std::string s);

class RegisterFile {
  public:
#define REG_CASE(...) MAKE_REGISTER(__VA_ARGS__),
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    RegisterClass<number_regs, reg_size> classname = {                         \
        #classname,                                                            \
        #reg_prefix,                                                           \
        {REGISTER_CLASS_##classname(REG_CASE)}};                               \
    template <typename T> void add##classname##ReadListener(T&& arg) {         \
        classname.addReadListener(std::forward<T>(arg));                       \
    }                                                                          \
    template <typename T> void add##classname##WriteListener(T&& arg) {        \
        classname.addWriteListener(std::forward<T>(arg));                      \
    }
#include "defs/registers.inc"

    template <typename T> void addReadListener(T&& arg) {
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    add##classname##ReadListener(std::forward<T>(arg));
#include "defs/registers.inc"
    }
    template <typename T> void addWriteListener(T&& arg) {
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    add##classname##WriteListener(std::forward<T>(arg));
#include "defs/registers.inc"
    }
};
}; // namespace rf
}; // namespace isa

#endif
