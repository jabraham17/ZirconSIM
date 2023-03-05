
#include "utils.h"

#include <algorithm>

namespace common {
namespace utils {
std::string toupper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return str;
}
std::string tolower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return str;
}
} // namespace utils
} // namespace common
