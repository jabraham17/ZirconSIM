#ifndef ZIRCON_ISHELL_GETLINE_H_
#define ZIRCON_ISHELL_GETLINE_H_

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace ishell {

struct getline {

    getline();
    getline(size_t history_size);
    getline(const std::string& history_file);
    getline(const std::string& history_file, size_t history_size);

    bool get(std::string& line, const std::string& prompt = "> ");

    using CompletionFunc =
        std::function<std::vector<std::string>(const std::string&)>;
    void addCompletetion(CompletionFunc func) {
        completionsForString.push_back(func);
    }

    std::vector<CompletionFunc> getCompletions() {
        return completionsForString;
    }

  private:
    std::filesystem::path historyFilePath;
    std::vector<CompletionFunc> completionsForString;
    std::string lastLine;
};
} // namespace ishell
#endif
