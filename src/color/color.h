
#ifndef ZIRCON_COLOR_COLOR_H_
#define ZIRCON_COLOR_COLOR_H_

#include <string>

namespace color {

std::string ESCAPE = "\033[";
std::string END = "m";

enum class ColorCode {
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    ORANGE = 33,
    BLUE = 34,
    PURPLE = 35,
    CYAN = 36,
    GREY = 37,

    DARK_GREY = 90,
    LIGHT_RED = 91,
    LIGHT_GREEN = 92,
    YELLOW = 93,
    LIGHT_BLUE = 94,
    TURQUOISE = 95,
    LIGHT_CYAN = 96,
    WHITE = 97,

    BLACK_BACKGROUND = 40,
    RED_BACKGROUND = 41,
    GREEN_BACKGROUND = 42,
    ORANGE_BACKGROUND = 43,
    BLUE_BACKGROUND = 44,
    PURPLE_BACKGROUND = 45,
    CYAN_BACKGROUND = 46,
    GREY_BACKGROUND = 47,

    DARK_GREY_BACKGROUND = 100,
    LIGHT_RED_BACKGROUND = 101,
    LIGHT_GREEN_BACKGROUND = 102,
    YELLOW_BACKGROUND = 103,
    LIGHT_BLUE_BACKGROUND = 104,
    TURQUOISE_BACKGROUND = 105,
    LIGHT_CYAN_BACKGROUND = 106,
    WHITE_BACKGROUND = 107,

    RESET = 0,
    BOLD = 1,
    UNDERLINE = 4,
    FLASH = 5,
    REVERSE = 7,
    INVISIBLE = 8,

};

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

#endif
