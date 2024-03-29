#include "format.h"

#include <iomanip>

namespace common {

namespace IOManip {
std::ostream& flush(std::ostream& o) {
    o.flush();
    return o;
}
} // namespace IOManip
namespace Format {

std::ostream& byte(std::ostream& o) {
    o << "0x" << std::setfill('0') << std::setw(2) << std::right << std::hex;
    return o;
}
std::ostream& halfword(std::ostream& o) {
    o << "0x" << std::setfill('0') << std::setw(4) << std::right << std::hex;
    return o;
}
std::ostream& word(std::ostream& o) {
    o << "0x" << std::setfill('0') << std::setw(8) << std::right << std::hex;
    return o;
}
std::ostream& doubleword(std::ostream& o) {
    o << "0x" << std::setfill('0') << std::setw(16) << std::right << std::hex;
    return o;
}
std::ostream& dec(std::ostream& o) {
    o << std::setfill(' ') << std::setw(2) << std::right << std::dec;
    return o;
}

std::ostream& operator<<(std::ostream& o, hexnum hf) {
    o << "0x" << std::setfill('0') << std::setw(hf.nBytes * 2) << std::right
      << std::hex;
    return o;
}

} // namespace Format
} // namespace common
