

#include "color/color.h"
#include "common/argparse.hpp"
#include "common/format.h"
#include "common/ordered_map.h"
#include "controller/parser/parser.h"
#include "cpu/cpu.h"
#include "cpu/isa/inst.h"
#include "elf/elf.h"
#include "event/event.h"
#include "mem/memory-image.h"
#include "trace/stats.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <unordered_map>

std::ostream* getFileStreamIfTrue(
    bool cond,
    std::optional<std::string> fname,
    std::ostream& alternative) {
    static std::unordered_map<std::string, std::ostream*> file_buffer;
    auto inst_log = &alternative;
    if(cond && fname) {
        // compare existing stats, if file already exists in buffer use that
        // ostream
        struct stat fname_info;
        auto stat_resno = stat(fname->c_str(), &fname_info);
        bool exists_in_file_buffer = false;
        if(stat_resno != 0) exists_in_file_buffer = false;

        auto it = std::find_if(
            file_buffer.begin(),
            file_buffer.end(),
            [fname_info](auto file_entry) {
                struct stat file_entry_info;
                if(stat(file_entry.first.c_str(), &file_entry_info) == 0) {
                    return fname_info.st_dev == file_entry_info.st_dev &&
                           fname_info.st_ino == file_entry_info.st_ino;
                }
                return false;
            });
        if(it != file_buffer.end()) {
            // return old file handle
            inst_log = it->second;
            exists_in_file_buffer = true;
        }

        if(!exists_in_file_buffer) {
            // open new file handle
            auto handle = (new std::ofstream(*fname));
            file_buffer[*fname] = handle;
            inst_log = handle;
        }
    }
    return inst_log;
}

// transforms string of form VAR=VALUE into pair {VAR,VALUE}
std::pair<std::string, std::string> splitVarEqualsValue(std::string s) {
    size_t pos = s.find('=');
    if(pos == std::string::npos) return {s, ""};
    else return {s.substr(0, pos), s.substr(pos + 1)};
}

int main(int argc, const char** argv, const char** envp) {

    argparse::ArgumentParser program_args(
        {},
        {},
        argparse::default_arguments::help);

    program_args.add_argument("file").help("elf64 file execute read from");

    program_args.add_argument("-c", "--color")
        .default_value(false)
        .implicit_value(true)
        .help("colorize output");

    program_args.add_argument("-I", "--inst")
        .default_value(false)
        .implicit_value(true)
        .help("trace instructions");
    program_args.add_argument("--inst-log")
        .metavar("LOGFILE")
        .help("instructions log file");

    program_args.add_argument("--csv")
        .default_value(false)
        .implicit_value(true)
        .help("enable csv output for supported traces (only supported for "
              "'--mem')");

    program_args.add_argument("-R", "--reg")
        .default_value(false)
        .implicit_value(true)
        .help("trace register accesses");
    program_args.add_argument("--reg-log")
        .metavar("LOGFILE")
        .help("register accesses log file");

    program_args.add_argument("-M", "--mem")
        .default_value(false)
        .implicit_value(true)
        .help("trace memory accesses");
    program_args.add_argument("--mem-log")
        .metavar("LOGFILE")
        .help("memory accesses log file");

    program_args.add_argument("--stats")
        .default_value(false)
        .implicit_value(true)
        .help("dump runtime statistics");

    program_args.add_argument("-control")
        .nargs(argparse::nargs_pattern::at_least_one)
        .append()
        .metavar("CONTROL")
        .help("a control sequence to apply\n\t\t\t  "
              "<subsystem>:<event> <conditions>? <actions>\n\t\t\t  "
              "WATCH <value> <actions>?\n\t\t\t");

    program_args.add_argument("-e", "-env")
        .append()
        .metavar("VAR=VALUE")
        .help("add environment variables");
    program_args.add_argument("-E", "--use-host-env")
        .default_value(false)
        .implicit_value(true)
        .help("pass the host environment variables to the simulated binary");

    program_args.add_argument("-S", "--syms")
        .default_value(false)
        .implicit_value(true)
        .help("try to resolve ELF symbols");

    // everything after -- gets shoved into the remaining args
    std::vector<std::string> simulated_argv;
    int newargc = argc;
    for(int i = 0; i < argc; i++) {
        if(argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == '\0') {
            newargc = i;
            for(i = i + 1; i < argc; i++) {
                simulated_argv.push_back(argv[i]);
            }
            break;
        }
    }

    try {
        program_args.parse_args(newargc, argv);
    } catch(const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program_args;
        return 1;
    }

    // check for controller args
    auto control_args = program_args.get<std::vector<std::string>>("-control");
    controller::ControlList parsed_commands;
    if(!control_args.empty()) {
        auto parser = controller::parser::Parser(control_args);
        try {
            parsed_commands = parser.parse();
        } catch(const controller::parser::ParseException& err) {
            std::cerr << "Invalid arguments for '-control': " << err.what()
                      << std::endl;
            std::cerr << program_args;
            return 1;
        }
    }

    std::string filename = program_args.get<std::string>("file");
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        std::cerr << "Failed to open '" << filename << "'" << std::endl;
        return 1;
    }
    // the filename is the first argv
    auto argv0 = filename;
    // FIXME: THIS CODE IS SO NASTY
    if(argv0.length() >= 2 && argv0[0] == '.' && argv0[1] == '/') {
        simulated_argv.insert(simulated_argv.begin(), argv0);
    } else if(argv0.length() >= 2 && argv0[0] == '/') {
        simulated_argv.insert(simulated_argv.begin(), argv0);
    } else {
        simulated_argv.insert(simulated_argv.begin(), "./" + argv0);
    }

    auto inst_log = getFileStreamIfTrue(
        program_args.get<bool>("--inst") && program_args.present("--inst-log"),
        program_args.present<std::string>("--inst-log"),
        std::cout);
    if(!inst_log) {
        std::cerr << "Failed to open '"
                  << *program_args.present<std::string>("--inst-log") << "'"
                  << std::endl;
        return 1;
    }

    auto mem_log = getFileStreamIfTrue(
        program_args.get<bool>("--mem") && program_args.present("--mem-log"),
        program_args.present<std::string>("--mem-log"),
        std::cout);
    if(!mem_log) {
        std::cerr << "Failed to open '"
                  << *program_args.present<std::string>("--mem-log") << "'"
                  << std::endl;
        return 1;
    }

    auto reg_log = getFileStreamIfTrue(
        program_args.get<bool>("--reg") && program_args.present("--reg-log"),
        program_args.present<std::string>("--reg-log"),
        std::cout);
    if(!reg_log) {
        std::cerr << "Failed to open '"
                  << *program_args.present<std::string>("--reg-log") << "'"
                  << std::endl;
        return 1;
    }

    elf::File f(std::move(is));
    cpu::Hart hart(std::make_shared<mem::MemoryImage>());
    f.buildMemoryImage(hart.hs().mem());
    auto start = f.getStartAddress();
    bool useColor = program_args.get<bool>("--color");

    auto elf_symbols = f.getSymbolTable();

    auto colorAddr = [useColor]() {
        return useColor ? color::getColor(
                              {color::ColorCode::LIGHT_CYAN,
                               color::ColorCode::FAINT})
                        : "";
    };
    auto colorHex = [useColor]() {
        return useColor ? color::getColor(
                              {color::ColorCode::CYAN, color::ColorCode::FAINT})
                        : "";
    };
    auto colorNew = [useColor]() {
        return useColor
                   ? color::getColor(
                         {color::ColorCode::GREEN, color::ColorCode::FAINT})
                   : "";
    };
    auto colorOld = [useColor]() {
        return useColor ? color::getColor(
                              {color::ColorCode::RED, color::ColorCode::FAINT})
                        : "";
    };
    auto colorSym = [useColor]() {
        return useColor ? color::getColor({color::ColorCode::GREEN}) : "";
    };
    auto colorReset = [useColor]() {
        return useColor ? color::getReset() : "";
    };

    if(program_args.get<bool>("--inst")) {
        if(program_args.get<bool>("--syms")) {
            hart.addBeforeExecuteListener([inst_log,
                                           useColor,
                                           colorReset,
                                           colorHex,
                                           colorAddr,
                                           colorSym,
                                           elf_symbols](cpu::HartState& hs) {
                auto inst = hs().getInstWord();

                *inst_log << "PC[" << colorAddr() << common::Format::doubleword
                          << hs().pc << colorReset() << "] = " << colorHex()
                          << common::Format::word << inst << colorReset()
                          << "; "
                          << isa::inst::disassemble(inst, hs().pc, useColor);
                auto it = elf_symbols.find(uint64_t(hs().pc));
                if(it != elf_symbols.end()) {
                    *inst_log << " <" << colorSym() << it->second << colorReset() << ">";
                }
                *inst_log << std::endl;
            });
        } else {
            hart.addBeforeExecuteListener(
                [inst_log, useColor, colorReset, colorHex, colorAddr](
                    cpu::HartState& hs) {
                    auto inst = hs().getInstWord();
                    *inst_log
                        << "PC[" << colorAddr() << common::Format::doubleword
                        << hs().pc << colorReset() << "] = " << colorHex()
                        << common::Format::word << inst << colorReset() << "; "
                        << isa::inst::disassemble(inst, hs().pc, useColor)
                        << std::endl;
                });
        }
    }

    if(program_args.get<bool>("--reg")) {
        hart.addRegisterReadListener([reg_log, colorReset, colorAddr, colorNew](
                                         std::string classname,
                                         uint64_t idx,
                                         uint64_t value) {
            *reg_log << "RD " << classname << "[" << colorAddr()
                     << common::Format::dec << idx << colorReset()
                     << "] = " << colorNew() << common::Format::doubleword
                     << (uint64_t)value << colorReset() << std::endl;
        });
        hart.addRegisterWriteListener(
            [reg_log, colorReset, colorAddr, colorNew, colorOld](
                std::string classname,
                uint64_t idx,
                uint64_t value,
                uint64_t oldvalue) {
                *reg_log << "WR " << classname << "[" << colorAddr()
                         << common::Format::dec << idx << colorReset()
                         << "] = " << colorNew() << common::Format::doubleword
                         << (uint64_t)value << colorReset()
                         << "; OLD VALUE = " << colorOld()
                         << common::Format::doubleword << oldvalue
                         << colorReset() << std::endl;
            });
    }
    if(program_args.get<bool>("--mem")) {
        if(!program_args.get<bool>("--csv")) {
            hart.hs().mem().addAllocationListener(
                [mem_log, colorReset, colorAddr](uint64_t addr, uint64_t size) {
                    *mem_log << "ALLOCATE[" << colorAddr()
                             << common::Format::doubleword << addr
                             << colorReset() << " - " << colorAddr()
                             << common::Format::doubleword << addr + size
                             << colorReset() << "]" << std::endl;
                });

            hart.hs().mem().addReadListener([mem_log, colorReset, colorAddr, colorNew](
                                       uint64_t addr,
                                       uint64_t value,
                                       size_t size) {
                *mem_log << "RD MEM[" << colorAddr()
                         << common::Format::doubleword << addr << colorReset()
                         << "] = " << colorNew() << common::Format::hexnum(size)
                         << (uint64_t)value << colorReset() << std::endl;
            });
            hart.hs().mem().addWriteListener(
                [mem_log, colorReset, colorAddr, colorNew, colorOld](
                    uint64_t addr,
                    uint64_t value,
                    uint64_t oldvalue,
                    size_t size) {
                    *mem_log << "WR MEM[" << colorAddr()
                             << common::Format::doubleword << addr
                             << colorReset() << "] = " << colorNew()
                             << common::Format::hexnum(size) << (uint64_t)value
                             << colorReset() << "; OLD VALUE = " << colorOld()
                             << common::Format::hexnum(size) << oldvalue
                             << colorReset() << std::endl;
                });
        } else {
            hart.hs().mem().addReadListener(
                [mem_log](uint64_t addr, uint64_t value, size_t size) {
                    *mem_log << "rd-mem," << addr << ",";
                    if(size == 1) *mem_log << (uint8_t)value;
                    else if(size == 2) *mem_log << (uint16_t)value;
                    else if(size == 4) *mem_log << (uint32_t)value;
                    else *mem_log << value;
                    *mem_log << std::endl;
                });
            hart.hs().mem().addWriteListener([mem_log](
                                        uint64_t addr,
                                        uint64_t value,
                                        [[maybe_unused]] uint64_t oldvalue,
                                        size_t size) {
                *mem_log << "wr-mem," << addr << ",";
                // need extra casts so that cout doesn't tray and interpret as a
                // char
                if(size == 1) *mem_log << (uint64_t)(uint8_t)value;
                else if(size == 2) *mem_log << (uint64_t)(uint16_t)value;
                else if(size == 4) *mem_log << (uint64_t)(uint32_t)value;
                else *mem_log << (uint64_t)value;
                *mem_log << std::endl;
            });
        }
    }
    Stats stats;
    if(program_args.get<bool>("--stats")) {
        hart.addBeforeExecuteListener(
            [&stats](cpu::HartState& hs) { stats.count(hs); });
    }

    for(auto a : parsed_commands.allActions()) {
        a->setHS(&hart.hs());
    }
    for(auto c : parsed_commands.allConditions()) {
        c->setHS(&hart.hs());
    }
    for(auto w : parsed_commands.watches) {
        w->setHS(&hart.hs());
        w->setLog(&std::cout);
    }
    for(auto c : parsed_commands.commands) {
        c->setColor(useColor);
        switch(c->getEventType()) {
            case event::EventType::HART_AFTER_EXECUTE:
                hart.addAfterExecuteListener(
                    [c](cpu::HartState&) { c->doit(&std::cout); });
                break;
            case event::EventType::HART_BEFORE_EXECUTE:
                hart.addBeforeExecuteListener(
                    [c](cpu::HartState&) { c->doit(&std::cout); });
                break;
            case event::EventType::MEM_READ:
                hart.hs().mem().addReadListener(
                    [c](uint64_t, uint64_t, size_t) { c->doit(&std::cout); });
                break;
            case event::EventType::MEM_WRITE:
                hart.hs().mem().addWriteListener(
                    [c](uint64_t, uint64_t, uint64_t, size_t) {
                        c->doit(&std::cout);
                    });
                break;
            case event::EventType::MEM_ALLOCATION:
                hart.hs().mem().addAllocationListener(
                    [c](uint64_t, uint64_t) { c->doit(&std::cout); });
                break;
            case event::EventType::REG_READ:
                hart.hs().rf().addReadListener(
                    [c](std::string, uint64_t, uint64_t) {
                        c->doit(&std::cout);
                    });
                break;
            case event::EventType::REG_WRITE:
                hart.hs().rf().addWriteListener(
                    [c](std::string, uint64_t, uint64_t, uint64_t) {
                        c->doit(&std::cout);
                    });
                break;
            default: std::cerr << "No Event Handler Defined\n";
        }
    }
    for(auto w : parsed_commands.watches) {
        w->setColor(useColor);
        hart.addBeforeExecuteListener([w](cpu::HartState&) { w->update(); });
        hart.addAfterExecuteListener([w](cpu::HartState&) { w->update(); });
    }

    // parse ennv
    common::ordered_map<std::string, std::string> env_vars;
    // std::map<std::string, std::string> env_vars;
    if(program_args.get<bool>("--use-host-env")) {
        for(int i = 0; envp[i] != 0; i++) {
            auto [key, value] = splitVarEqualsValue(envp[i]);
            env_vars.insert_or_assign(key, value);
        }
    }
    for(auto e : program_args.get<std::vector<std::string>>("-env")) {
        auto [key, value] = splitVarEqualsValue(e);
        env_vars.insert_or_assign(key, value);
    }

    hart.init(simulated_argv, env_vars);
    hart.execute(start);

    if(program_args.get<bool>("--stats")) {
        std::cout << stats.dump() << std::endl;
    }

    return 0;
}
