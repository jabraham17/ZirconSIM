#include "color.h"

namespace color {
std::string ESCAPE = "\033[";
std::string END = "m";

std::string combine(std::initializer_list<ColorCode> codes) {
    std::string combined = "";
    std::string sep;
    for(auto c : codes) {
        combined += sep + std::to_string(int(c));
        sep = ";";
    }
    return combined;
}

std::string getColor(std::initializer_list<ColorCode> codes) {
    return ESCAPE + combine(codes) + END;
}

std::string getReset() { return getColor({ColorCode::RESET}); }

std::string getWrapped(std::string a, std::initializer_list<ColorCode> codes) {
    return getColor(codes) + a + getReset();
}

} // namespace color
