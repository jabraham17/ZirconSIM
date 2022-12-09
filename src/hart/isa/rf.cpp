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
} // namespace rf
} // namespace isa
