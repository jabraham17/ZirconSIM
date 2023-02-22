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

std::optional<RegisterSymbol> parseRegister(std::string s) {
#define REG_CASE(classname, index, nice_name, ...)                             \
    if(s == #nice_name ||                                                      \
       s == internal::registerPrefixFor##classname + std::to_string(index))    \
        return RegisterSymbol(getRegisterClassType(#classname), index);
#define REGISTER_CLASS(classname, ...) REGISTER_CLASS_##classname(REG_CASE)
#include "defs/registers.inc"
#undef REG_CASE2

    // failed to parse a register
    return std::nullopt;
}
} // namespace rf
} // namespace isa
