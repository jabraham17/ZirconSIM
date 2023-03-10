#ifndef ZIRCON_ISHELL_GETLINE_H_
#define ZIRCON_ISHELL_GETLINE_H_

#include <string>

namespace ishell {
    namespace getline {

        struct History {

        };
        
        // returns the next line with no trailing newline
        // returns empty string if EOF
        std::string getline();
        // sets `line` with no trailing newline, returns false on EOF
        bool getline(std::string& line);

        // returns the next line with no trailing newline
        // returns empty string if EOF
        // allows user to search a history with arrow keys, function will update history
        std::string getline(History& history);
        // sets `line` with no trailing newline, returns false on EOF
        // allows user to search a history with arrow keys, function will update history
        bool getline(std::string& line, History& history);

    }
}
#endif
