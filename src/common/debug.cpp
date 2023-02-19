#include "debug.h"

#include "utils.h"

namespace common {
namespace debug {

#define TOKEN(d, v) const DebugType DebugType::d = v;
DEBUG_CATAGORIES(TOKEN)
#undef TOKEN

// OR all of the debug types together
static auto getAllValue() {
#define DEBUG_CASE(d, v) v |
    return DEBUG_CATAGORIES(DEBUG_CASE) 0;
#undef DEBUG_CASE
}

DebugType::DebugType(std::string s) : value_() {
    std::string S = utils::toupper(s);
    if(S == "ALL") {
        this->value_ = getAllValue();
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

CancelableOStream rawlog(DebugType dt, std::ostream& os) {
    bool isEnabled = checkDebugState(dt);
    return CancelableOStream(isEnabled, os);
}
CancelableOStream rawlog(DebugType dt) { return rawlog(dt, std::cout); }
CancelableOStream rawlog(std::ostream& os) {
    return rawlog(DebugType::LOG, os);
}
CancelableOStream rawlog() { return rawlog(std::cout); }

} // namespace debug
} // namespace common
