#ifndef ZIRCON_COMMON_FORMAT_H_
#define ZIRCON_COMMON_FORMAT_H_

#include <ostream>

namespace common {

namespace IOManip {
std::ostream& flush(std::ostream& o);
} // namespace IOManip
namespace Format {

std::ostream& byte(std::ostream& o);
std::ostream& halfword(std::ostream& o);
std::ostream& word(std::ostream& o);
std::ostream& doubleword(std::ostream& o);
std::ostream& dec(std::ostream& o);
struct hexnum {
    size_t nBytes;
    hexnum(size_t nBytes) : nBytes(nBytes) {}
    friend std::ostream& operator<<(std::ostream& o, hexnum hf);
};
} // namespace Format
} // namespace common

#endif
