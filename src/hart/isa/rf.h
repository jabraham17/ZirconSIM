#ifndef ZIRCON_HART_ISA_RF_H_
#define ZIRCON_HART_ISA_RF_H_

#include "register.h"

#include "event/event.h"
#include "hart/types.h"

#include <optional>

namespace isa {
namespace rf {

struct IllegalRegisterClassException : public std::runtime_error {
    IllegalRegisterClassException() : std::runtime_error("Illegal Register Class") {}
    IllegalRegisterClassException(std::string s)
        : std::runtime_error("Illegal Register Class " + s) {}
};
struct IllegalRegisterException : public std::runtime_error {
    IllegalRegisterException() : std::runtime_error("Illegal Register") {}
    IllegalRegisterException(std::string s)
        : std::runtime_error("Illegal Register " + s) {}
};

enum class RegisterClassType {
#define REGISTER_CLASS(r, ...) r,
#include "defs/registers.inc"
    NONE,
};
// a type to refer to a register
using RegisterSymbol = std::pair<RegisterClassType, types::RegisterIndex>;

bool isRegisterClassType(std::string s);
RegisterClassType getRegisterClassType(std::string s);
std::string getRegisterClassString(RegisterClassType rcf);
std::optional<RegisterSymbol>
parseRegister(std::string s);

namespace internal {
#define REGISTER_CLASS(classname, reg_prefix, ...)                             \
    const std::string registerPrefixFor##classname = #reg_prefix;
#include "defs/registers.inc"
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    inline constexpr size_t registerSizeFor##classname = reg_size;
#include "defs/registers.inc"
} // namespace internal

class RegisterFile {
  public:
#define REG_CASE(...) MAKE_REGISTER(__VA_ARGS__),
#define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)           \
    RegisterClass<number_regs, internal::registerSizeFor##classname>           \
        classname = {                                                          \
            #classname,                                                        \
            #reg_prefix,                                                       \
            {REGISTER_CLASS_##classname(REG_CASE)}};                           \
    template <typename T> void add##classname##ReadListener(T&& arg) {         \
        classname.addReadListener(std::forward<T>(arg));                       \
    }                                                                          \
    template <typename T> void add##classname##WriteListener(T&& arg) {        \
        classname.addWriteListener(std::forward<T>(arg));                      \
    }
#include "defs/registers.inc"
#undef REG_CASE

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
    
    auto getRegisterClassForType(RegisterClassType rct) {
        auto rct_str = getRegisterClassString(rct);
        #define REGISTER_CLASS(classname, ...) if(rct_str == #classname) return classname;
        #include "defs/registers.inc"
        throw IllegalRegisterClassException(rct_str);
    }
};
}; // namespace rf
}; // namespace isa

#endif
