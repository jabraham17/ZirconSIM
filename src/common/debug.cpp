#include "debug.h"

#include "utils.h"

namespace common {
namespace debug {

#define TOKEN(d, v) const DebugType DebugType::d = v;
    DEBUG_CATAGORIES(TOKEN)
#undef TOKEN

DebugType::DebugType(std::string s) : value_() {
    std::string S = utils::toupper(s);
    if(0) {
    }
#define DEBUG_CASE(d, v) else if(S == #d) this->value_ = v;
    DEBUG_CATAGORIES(DEBUG_CASE)
#undef DEBUG_CASE
}

DebugType CURRENT_DEBUG_STATE;
bool checkDebugState(DebugType dt) { return dt & CURRENT_DEBUG_STATE; }
void setDebugState(DebugType dt) { CURRENT_DEBUG_STATE = dt; }
void updateDebugState(DebugType dt) { CURRENT_DEBUG_STATE |= dt; }
DebugType getDebugState() { return CURRENT_DEBUG_STATE; }
} // namespace debug
} // namespace common
