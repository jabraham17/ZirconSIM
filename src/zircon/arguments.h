#ifndef ZIRCON_ZIRCON_ARGUMENTS_H_
#define ZIRCON_ZIRCON_ARGUMENTS_H_

#include "command/command.h"
#include "common/argparse.hpp"
#include "common/ordered_map.h"
#include "elf/elf.h"
#include "hart/hart.h"

#include <exception>
#include <fstream>
#include <iostream>

namespace arguments {

struct ArgumentException : public std::runtime_error {
    ArgumentException() : std::runtime_error("Unknown Argument Exception") {}
    ArgumentException(std::string message) : std::runtime_error(message) {}
};

class MainArguments {
  public:
    void parse(int argc, const char** argv, const char** envp = nullptr);
    const argparse::ArgumentParser& accessRawArguments() {
        return program_args;
    }
    std::ifstream getInputFile();
    void addCallbacks(hart::Hart& hart, elf::File& elf);
    void addControllerCallbacks(hart::Hart& hart);
    std::vector<std::string> getArgV();
    common::ordered_map<std::string, std::string> getEnvVars();

    // check if we should use color or not
    bool useColor();

  private:
    MainArguments();
    argparse::ArgumentParser program_args;
    std::vector<std::string> simulated_argv;
    common::ordered_map<std::string, std::string> simulated_env;

    std::ifstream* input;
    std::ostream* inst_log;
    std::ostream* mem_log;
    std::ostream* reg_log;

    std::vector<command::CommandPtr> parsed_commands;

  public:
    static MainArguments getMainArguments();
};

} // namespace arguments
#endif
