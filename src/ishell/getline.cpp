#include "getline.h"

#include "common/utils.h"
#include "linenoise/linenoise.h"

#include <string.h>
#include <string>
#include <strings.h>
#include <wordexp.h>

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

extern "C" void
completion_function(void* arg, const char* buf, linenoiseCompletions* lc) {
    auto this_ = (getline*)(arg);
    std::string entered(buf);
    for(auto cf : this_->getCompletions()) {
        auto comps = cf(entered);
        for(auto c : comps) {
            linenoiseAddCompletion(lc, c.c_str());
        }
    }
}
// extern "C" char* hints_function(void* arg, const char *buf, int*color, int*
// bold) {
//     auto this_ = (getline*)(arg);
//     std::string entered(buf);
//     *color = 35;
//     *bold = 0;

//     // hint the args a tab would give
//     std::vector<std::string> hints;
//     for(auto cf: this_->getCompletions()) {
//         auto comps = cf(entered);
//         hints.insert(hints.end(), comps.begin(), comps.end());
//     }
//     std::string hint = common::utils::join(hints.begin(), hints.end(), " ");
//     char* hint_cstr = strdup(hint.c_str());
//     return hint_cstr;
// }

getline::getline() : getline(DEFAULT_HISTORY_FILE, DEFAULT_HISTORY_SIZE) {}
getline::getline(size_t history_size)
    : getline(DEFAULT_HISTORY_FILE, history_size) {}
getline::getline(const std::string& history_file)
    : getline(history_file, DEFAULT_HISTORY_SIZE) {}

getline::getline(const std::string& history_file, size_t history_size)
    : historyFilePath(expandPath(history_file)), completionsForString(),
      lastLine() {
    std::filesystem::create_directories(historyFilePath.parent_path());

    linenoiseHistoryLoad(historyFilePath.c_str());
    linenoiseHistorySetMaxLen(history_size);
    linenoiseSetMultiLine(1);
}

bool getline::get(std::string& line, const std::string& prompt) {

    // TODO ADD COMPLETION
    linenoiseSetCompletionCallback(completion_function, (void*)this);
    // linenoiseSetHintsCallback(hints_function, (void*)this);

    char* c_line = linenoise(prompt.c_str());
    if(c_line == NULL) return false;

    line = std::string(c_line);

    // use lastLine if line is empty
    if(line == "") line = lastLine;

    // only add to history if different than last line
    if(line != "" && line != lastLine) {
        lastLine = line;
        linenoiseHistoryAdd(line.c_str());
        linenoiseHistorySave(historyFilePath.c_str());
    }
    linenoiseFree(c_line);

    return true;
}

} // namespace ishell
