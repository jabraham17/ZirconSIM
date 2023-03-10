#include "getline.h"

namespace ishell {

namespace getline {

namespace internal {
// main driver for impl
extern bool getline_impl(std::string& line, History* history);

}
std::string getline() {
    std::string line;
    auto res = internal::getline_impl(line, nullptr);
    if(!res) line = "";
    return line;
}
bool getline(std::string& line) { return internal::getline_impl(line, nullptr); }

std::string getline(History& history) {
    std::string line;
    auto res = internal::getline_impl(line, &history);
    if(!res) line = "";
    return line;
}
bool getline(std::string& line, History& history) {
    return internal::getline_impl(line, &history);
}

} // namespace getline
} // namespace ishell
