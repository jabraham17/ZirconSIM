#include "rf.h"

namespace isa {
namespace rf {
bool isRegisterClassType(std::string s) {
#define REGISTER_CLASS(r, ...)                                                 \
    if(s == #r) return true;
#include "defs/registers.inc"
    return false;
}
RegisterClassType getRegisterClassType(std::string s) {
#define REGISTER_CLASS(r, ...)                                                 \
    if(s == #r) return RegisterClassType::r;
#include "defs/registers.inc"
    return RegisterClassType::NONE;
}
std::string getRegisterClassString(RegisterClassType rcf) {
#define REGISTER_CLASS(r, ...)                                                 \
    if(rcf == RegisterClassType::r) return #r;
#include "defs/registers.inc"
    return "NONE";
}

// // functions to lookup register prefix per classname
// namespace internal {
// #define REGISTER_CLASS(classname, reg_prefix, ...) \
//     const std::string registerPrefixFor##classname = #reg_prefix;
// #include "defs/registers.inc"

// // functions to lookup register prefix per classname
// #define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size) \
//     constexpr size_t registerSizeFor##classname = reg_size;
// #include "defs/registers.inc"

// } // namespace internal

std::optional<std::pair<RegisterClassType, types::RegisterIndex>>
parseRegister(std::string s) {
#define REG_CASE2(classname, index, nice_name, ...)                            \
    if(s == #nice_name ||                                                      \
       s == internal::registerPrefixFor##classname + std::to_string(index))    \
        return std::make_pair(getRegisterClassType(#classname), index);
#define REG_CASE(...) REG_CASE2(__VA_ARGS__)
#define REGISTER_CLASS(classname, ...) REGISTER_CLASS_##classname(REG_CASE)
#include "defs/registers.inc"
#undef REG_CASE2
#undef REG_CASE

    // failed to parse a register
    return std::nullopt;
}
} // namespace rf
} // namespace isa
