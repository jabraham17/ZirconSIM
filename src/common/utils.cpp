
#include "utils.h"

#include <algorithm>
#include <iterator>

namespace common {
namespace utils {
std::string toupper(const std::string& str) {
    std::string r;
    std::transform(
        str.begin(),
        str.end(),
        std::back_inserter(r),
        [](unsigned char c) { return std::toupper(c); });
    return r;
}
std::string tolower(const std::string& str) {
    std::string r;
    std::transform(
        str.begin(),
        str.end(),
        std::back_inserter(r),
        [](unsigned char c) { return std::tolower(c); });
    return r;
}
std::vector<std::string> split(const std::string& str, const std::string& sep) {
    std::vector<std::string> tokens;
    size_t pos_start = 0, pos_end, sep_len = sep.length();

    while((pos_end = str.find(sep, pos_start)) != std::string::npos) {
        auto token = str.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + sep_len;
        tokens.push_back(token);
    }

    tokens.push_back(str.substr(pos_start));
    return tokens;
}
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.find(prefix, 0) == 0;
}

} // namespace utils
} // namespace common
