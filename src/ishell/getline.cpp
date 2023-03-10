#include "getline.h"

#include <wordexp.h>
#include <strings.h>
#include "linenoise/linenoise.h"

namespace ishell {

static std::string expandPath(std::string s) {
    std::string res;
    wordexp_t exp_result;
    wordexp(s.c_str(), &exp_result, 0);
    res = std::string(exp_result.we_wordv[0]);
    wordfree(&exp_result);
    return res;
}
static const std::string DEFAULT_HISTORY_FILE = "~/.zircon/history";
static const size_t DEFAULT_HISTORY_SIZE = 50;

getline::getline() : getline(DEFAULT_HISTORY_FILE, DEFAULT_HISTORY_SIZE) {}
getline::getline(size_t history_size)
    : getline(DEFAULT_HISTORY_FILE, history_size) {}
getline::getline(const std::string& history_file)
    : getline(history_file, DEFAULT_HISTORY_SIZE) {}

getline::getline(const std::string& history_file, size_t history_size)
    : historyFilePath(expandPath(history_file)), completionsForString(), lastLine() {
        std::filesystem::create_directories(historyFilePath.parent_path());

        linenoiseHistoryLoad(historyFilePath.c_str());
        linenoiseHistorySetMaxLen(history_size);
        linenoiseSetMultiLine(1);
    }

    bool getline::get(std::string& line, const std::string& prompt) {
        char* c_line = linenoise(prompt.c_str());
        if(c_line == NULL) return false;

        line = std::string (c_line);

        //only add to history if diffeent than last line
        if(line != "" && line != lastLine) {
            lastLine = line;
            linenoiseHistoryAdd(line.c_str());
            linenoiseHistorySave(historyFilePath.c_str());

        }
        free(c_line);

        return true;
    }

} // namespace ishell
