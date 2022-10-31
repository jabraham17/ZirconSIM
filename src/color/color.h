
#ifndef ZIRCON_COLOR_COLOR_H_
#define ZIRCON_COLOR_COLOR_H_

#include <string>

namespace color {
// TODO: finish fully implemention
// https://chrisyeh96.github.io/2020/03/28/terminal-colors.html
// https://robotmoon.com/256-colors/

extern std::string ESCAPE;
extern std::string END;

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
    FAINT = 2,
    UNDERLINE = 4,
    FLASH = 5,
    REVERSE = 7,
    INVISIBLE = 8,
};

std::string combine(std::initializer_list<ColorCode> codes);

std::string getColor(std::initializer_list<ColorCode> codes);
std::string getReset();

std::string getWrapped(std::string a, std::initializer_list<ColorCode> codes);
} // namespace color

#endif
