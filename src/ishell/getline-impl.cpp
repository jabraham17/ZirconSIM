#include "getline.h"

#include <cassert>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace ishell {
namespace getline {

namespace internal {

bool isTerminalInput() { return isatty(fileno(stdin)); }
bool isTerminalOutput() { return isatty(fileno(stdout)); }

void saveTermState(termios* save) { tcgetattr(fileno(stdin), save); }
void restoreTermState(termios* saved) {
    tcsetattr(fileno(stdin), TCSANOW, saved);
}
void setTermReadlineState(const termios* initial) {
    termios state = *initial;
    // disable canonical mode: handle char by char
    // do not echo: we will do that manually
    state.c_lflag &= ~(ICANON | ECHO | ECHOE);
    state.c_cc[VMIN] = 1;
    state.c_cc[VTIME] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &state);
}


char ESC = '\033';
char CSI = '[';

// should never contain ESC
std::vector<char> input_buffer;

// gets next DSR report, buffering all other chars
std::string getNextDSR() {
    // if(!input_buffer.empty()) {
    //     auto it = input_buffer.begin();
    //     for(; it != input_buffer.end(); it++) {
    //         if(*it == ESC) return ESC;
    //     }
    // }

    std::string s;
    bool found = false;
    char c = 0;
    // input buffer exhausted, run read loop
    while(1) {
        read(fileno(stdin), &c, 1);
        if(c == 0) return s;
        if(c == ESC && !found) {
            found = true;
        }

        if(found) {
            s += c;
            if(c == 'R') return s;
        }
        else {
        input_buffer.push_back(c);
        }
    }
}

char getChar() {
    char c;
    if(input_buffer.empty())
        read(fileno(stdin), &c, 1);
    else {
        c = input_buffer[0];
        input_buffer.erase(input_buffer.begin());
    }
    // char c = (char)getchar();
    return c;
}


// // consumes the `through`
// // returns n chars read, including through
// size_t readThroughInBuffer(char through, char* buffer, size_t buffer_size) {
//     assert(buffer != nullptr && buffer_size > 0);
//     size_t i;
//     for(i = 0; i < (buffer_size - 1); i++) {
//         char c = (char)getChar();
//         if()
//     }
// }

std::pair<size_t, size_t> getScreenSize();

namespace cursor {
void moveInDirection(size_t i, char dir) {
    assert(i > 0 && (dir == 'A' || dir == 'B' || dir == 'C' || dir == 'D'));
    std::string s;
    s += ESC;
    s += CSI;
    s += std::to_string(i);
    s += dir;
    write(fileno(stdout), s.c_str(), s.size());
    fsync(fileno(stdout));
    // std::cout << s;
}
void goLeft(size_t i = 1) { moveInDirection(i, 'D'); }
void goRight(size_t i = 1) { moveInDirection(i, 'C'); }
void goUp(size_t i = 1) { moveInDirection(i, 'A'); }
void goDown(size_t i = 1) { moveInDirection(i, 'B'); }
void setCursor(size_t n, size_t m) {
    std::string s;
    s += ESC;
    s += CSI;
    s += std::to_string(n);
    s += ';';
    s += std::to_string(m);
    s += 'H';
    write(fileno(stdout), s.c_str(), s.size());
    fsync(fileno(stdout));
    // std::cout.flush();
    // std::cout << s;
    // std::cout.flush();
}
std::pair<std::string, std::string> splitOnSep(std::string s, char sep) {
    size_t pos = s.find(sep);
    if(pos == std::string::npos) return {s, ""};
    else return {s.substr(0, pos), s.substr(pos + 1)};
}
std::string readThrough(char marker) {
    std::string s;
    char c;
    while((c = getChar()) != 0 && c != marker) {
        s+=c;
    }
    return s;
}
void setCursor(std::pair<size_t, size_t> c) { setCursor(c.first, c.second); }
std::pair<size_t, size_t> getCursor() {
    std::cout.flush();
    char buf[] = {ESC, CSI, '6', 'n', 0};
    write(fileno(stdout), buf, 4);
    fsync(fileno(stdout));
    // std::cout << buf;
    // std::cout.flush();
    std::string dsr = getNextDSR();
    // chop of ESC CSI and 'R'
    dsr = dsr.substr(2, dsr.size() - 3);
            // std::string s = readThrough('R');
            auto [nStr, mStr] = splitOnSep(dsr, ';');
            size_t n = std::stoull(nStr);
            size_t m = std::stoull(mStr);
            return {n, m};

    return {0, 0};
}
// this is the manhattan/euclidean distance
size_t countCharsBetweenCursors(
    std::pair<size_t, size_t> start,
    std::pair<size_t, size_t> end = getCursor()) {
    return std::llabs(ssize_t(end.first) - ssize_t(start.first)) +
           std::llabs(ssize_t(end.second) - ssize_t(end.second));
}

std::pair<size_t, size_t> cursorAdd(std::pair<size_t, size_t> p, size_t n) {
    auto size = getScreenSize();
    p.second += n;
    if(p.second > size.second) {
        auto diff = p.second-size.second;
        p.first+= p.second-size.second;
        p.second = n - diff;
        
        char buf[] = {ESC, CSI, '1', 'S', 0};
        write(fileno(stdout), buf, 4);
    }
    // auto mod = n % size.second;
    // p.first += n / size.second;
    // p.second += mod;
    // p.second += 1;
    return p;
}

} // namespace cursor

std::pair<size_t, size_t> getScreenSize() {
    auto cur = cursor::getCursor();
    cursor::setCursor({999, 999});
    auto size = cursor::getCursor();
    cursor::setCursor(cur);
    return size;
}

size_t bounded0Subtract(size_t n, size_t s) {
    if(n < s) return 0;
    else return n - s;
}

bool getline_impl(std::string& line, History* history) {
    if(!isTerminalInput()) {
        std::getline(std::cin, line);
        return !std::cin.eof(); // return true if not EOF
    }
    termios saved_state;
    saveTermState(&saved_state);
    setTermReadlineState(&saved_state);

    auto size = getScreenSize();
    std::cerr << size.first << "," << size.second << "\n";

    // auto line_start = cursor::getCursor();

    std::vector<char> line_chars;
    size_t insert_point = 0;

    char c;
    while((c = getChar()) != 0) {
        // std::cerr << std::dec << (int)c << std::endl;
        if(c == '\n') {
            char buf[] = {'\n', 0};
            write(fileno(stdout), buf, 1);
            fsync(fileno(stdout));
            break;
        } else if(c == ESC) {
            if(getChar() == CSI) {
                char control = getChar();
                if(control == 'D') {
                    cursor::goLeft();
                    insert_point = bounded0Subtract(insert_point, 1);
                }
            }
        } else if(c == 0x7F) {
            if(insert_point > 0) {
                char buf[] = {'\b', ' ', '\b', 0};
                write(fileno(stdout), buf, 3);
                fsync(fileno(stdout));
                // std::cout << "\b \b";
                insert_point = bounded0Subtract(insert_point, 1);
                line_chars.erase(line_chars.begin() + insert_point);
                if(insert_point + 1 < line_chars.size())
                    std::cout << std::string(
                        line_chars.begin() + insert_point + 1,
                        line_chars.end());
            }
            // insert_point;
        } else if(isprint(c)) {
            auto curs = cursor::getCursor();
            // std::cerr << "CURSOR: " << curs.first << "," << curs.second <<
            // "\n"; size_t i = cursor::countCharsBetweenCursors(line_start,
            // curs); std::cerr << i << std::endl;
            line_chars.insert(line_chars.begin() + insert_point, c);
            // reprint from the insert pos to the end, this simulates insertion
            std::string s(
                line_chars.begin() + insert_point,
                line_chars.end());
                write(fileno(stdout), s.c_str(), s.size());
                fsync(fileno(stdout));
            insert_point++;
            curs = cursor::cursorAdd(curs, 1);
            std::cerr << "CURSOR: " << curs.first << "," << curs.second <<
            "\n";
            cursor::setCursor(curs);
        }
    }

    // std::cout.flush();
    line = std::string(line_chars.begin(), line_chars.end());

    restoreTermState(&saved_state);
    return !std::cin.eof(); // return true if not EOF
}

} // namespace internal
} // namespace getline
} // namespace ishell
