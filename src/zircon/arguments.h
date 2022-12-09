#ifndef ZIRCON_ZIRCON_ARGUMENTS_H_
#define ZIRCON_ZIRCON_ARGUMENTS_H_

#include "common/argparse.hpp"
#include "cpu/cpu.h"
#include "common/ordered_map.h"
#include <iostream>
#include <fstream>
#include "elf/elf.h"
#include "controller/command.h"

namespace arguments {


class MainArguments {
    public:
    void parse(int argc, const char** argv, const char** envp = nullptr);
    const argparse::ArgumentParser& accessRawArguments() {return program_args;}
    std::ifstream& getInputFile();
    void addCallbacks(cpu::Hart& hart, elf::File& elf);
    void addControllerCallbacks(cpu::Hart& hart);
    std::vector<std::string> getArgV();
    common::ordered_map<std::string, std::string> getEnvVars();

    private:
    MainArguments();
    argparse::ArgumentParser program_args;
    std::vector<std::string> simulated_argv;
        common::ordered_map<std::string, std::string> simulated_env;

    std::ifstream* input;
    std::ostream* inst_log;
    std::ostream* mem_log;
    std::ostream* reg_log;

    controller::ControlList parsed_commands;


    public:
    static MainArguments getMainArguments();

};


}
#endif
