#ifndef ZIRCON_COMMON_UTILS_H_
#define ZIRCON_COMMON_UTILS_H_
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

namespace common {
namespace utils {
extern std::string toupper(const std::string& str);
extern std::string tolower(const std::string& str);
extern std::vector<std::string>
split(const std::string& str, const std::string& sep = " ");
template <class ForwardIT>
std::string
join(ForwardIT elms_begin, ForwardIT elms_end, const std::string& join = "") {
    std::string sep;
    std::string res;
    for(auto it = elms_begin; it != elms_end; it++) {
        res = res + sep + *it;
        sep = join;
    }
    return res;
}
template <class ForwardIT>
std::string join(
    ForwardIT elms_begin,
    ForwardIT elms_end,
    std::function<
        std::string(typename std::iterator_traits<ForwardIT>::value_type)> func,
    const std::string& join = "") {
    std::string sep;
    std::string res;
    for(auto it = elms_begin; it != elms_end; it++) {
        res = res + sep + func(*it);
        sep = join;
    }
    return res;
}
extern bool startsWith(const std::string& str, const std::string& starts);

} // namespace utils
} // namespace common
#endif
