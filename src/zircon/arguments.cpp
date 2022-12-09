#include "arguments.h"

#include "color/color.h"
#include "common/format.h"
#include "controller/parser/parser.h"
#include "event/event.h"
#include "hart/isa/inst.h"

#include <algorithm>
#include <iomanip>
#include <sys/stat.h>
#include <unordered_map>

namespace arguments {
std::ostream* getFileStreamIfTrue(
    bool cond,
    std::optional<std::string> fname,
    std::ostream& alternative) {
    static std::unordered_map<std::string, std::ostream*> file_buffer;
    auto handle_to_return = &alternative;
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
            handle_to_return = it->second;
            exists_in_file_buffer = true;
        }

        if(!exists_in_file_buffer) {
            // open new file handle
            auto handle = (new std::ofstream(*fname));
            file_buffer[*fname] = handle;
            handle_to_return = handle;
        }
    }
    return handle_to_return;
}

// transforms string of form VAR=VALUE into pair {VAR,VALUE}
std::pair<std::string, std::string> splitVarEqualsValue(std::string s) {
    size_t pos = s.find('=');
    if(pos == std::string::npos) return {s, ""};
    else return {s.substr(0, pos), s.substr(pos + 1)};
}

auto colorAddr(bool useColor) {
    return useColor
               ? color::getColor(
                     {color::ColorCode::LIGHT_CYAN, color::ColorCode::FAINT})
               : "";
}
auto colorHex(bool useColor) {
    return useColor ? color::getColor(
                          {color::ColorCode::CYAN, color::ColorCode::FAINT})
                    : "";
}
auto colorNew(bool useColor) {
    return useColor ? color::getColor(
                          {color::ColorCode::GREEN, color::ColorCode::FAINT})
                    : "";
}
auto colorOld(bool useColor) {
    return useColor ? color::getColor(
                          {color::ColorCode::RED, color::ColorCode::FAINT})
                    : "";
}
auto colorSym(bool useColor) {
    return useColor ? color::getColor({color::ColorCode::GREEN}) : "";
}
auto colorReset(bool useColor) { return useColor ? color::getReset() : ""; }

MainArguments::MainArguments()
    : program_args({}, {}, argparse::default_arguments::help) {
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
}

void MainArguments::parse(int argc, const char** argv, const char** envp) {
    // everything after -- gets shoved into the remaining args
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
        throw ArgumentException("Bad arguments: " +
            std::string(err.what()) + "\n" + program_args.help().str());
    }

    std::string filename = program_args.get<std::string>("file");
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

    // parse env
    if(envp && program_args.get<bool>("--use-host-env")) {
        for(int i = 0; envp[i] != 0; i++) {
            auto [key, value] = splitVarEqualsValue(envp[i]);
            simulated_env.insert_or_assign(key, value);
        }
    }
    for(auto e : program_args.get<std::vector<std::string>>("-env")) {
        auto [key, value] = splitVarEqualsValue(e);
        simulated_env.insert_or_assign(key, value);
    }

    // check for controller args
    auto control_args = program_args.get<std::vector<std::string>>("-control");
    if(!control_args.empty()) {
        auto parser = controller::parser::Parser(control_args);
        try {
            parsed_commands = parser.parse();
        } catch(const controller::parser::ParseException& err) {
            throw ArgumentException(
                "Invalid arguments for '-control': " + std::string(err.what()) + "\n" + program_args.help().str());
        }
    }

    input = (new std::ifstream(filename, std::ios::binary));
    if(!input) {
        throw ArgumentException("Failed to open '" + filename + "'");
    }

    auto inst_log = getFileStreamIfTrue(
        program_args.get<bool>("--inst") && program_args.present("--inst-log"),
        program_args.present<std::string>("--inst-log"),
        std::cout);
    if(!inst_log) {
        throw ArgumentException(
            "Failed to open '" +
            *program_args.present<std::string>("--inst-log") + "'");
    }

    auto mem_log = getFileStreamIfTrue(
        program_args.get<bool>("--mem") && program_args.present("--mem-log"),
        program_args.present<std::string>("--mem-log"),
        std::cout);
    if(!mem_log) {
        throw ArgumentException(
            "Failed to open '" +
            *program_args.present<std::string>("--mem-log") + "'");
    }

    auto reg_log = getFileStreamIfTrue(
        program_args.get<bool>("--reg") && program_args.present("--reg-log"),
        program_args.present<std::string>("--reg-log"),
        std::cout);
    if(!reg_log) {
        throw ArgumentException(
            "Failed to open '" +
            *program_args.present<std::string>("--reg-log") + "'");
    }
}

std::ifstream MainArguments::getInputFile() {
    if(input) return std::move(*input);
    throw ArgumentException("No valid input file");
}
void MainArguments::addCallbacks(hart::Hart& hart, elf::File& elf) {

    bool useColor = program_args.get<bool>("--color");
    auto elf_symbols = elf.getSymbolTable();

    if(program_args.get<bool>("--inst")) {
        if(program_args.get<bool>("--syms")) {
            hart.addBeforeExecuteListener([this,
                                           useColor,

                                           elf_symbols](hart::HartState& hs) {
                auto inst = hs().getInstWord();

                *this->inst_log
                    << "PC[" << colorAddr(useColor)
                    << common::Format::doubleword << hs().pc
                    << colorReset(useColor) << "] = " << colorHex(useColor)
                    << common::Format::word << inst << colorReset(useColor)
                    << "; " << isa::inst::disassemble(inst, hs().pc, useColor);
                auto it = elf_symbols.find(uint64_t(hs().pc));
                if(it != elf_symbols.end()) {
                    *this->inst_log << " <" << colorSym(useColor) << it->second
                                    << colorReset(useColor) << ">";
                }
                *this->inst_log << std::endl;
            });
        } else {
            hart.addBeforeExecuteListener([this,
                                           useColor](hart::HartState& hs) {
                auto inst = hs().getInstWord();
                *this->inst_log
                    << "PC[" << colorAddr(useColor)
                    << common::Format::doubleword << hs().pc
                    << colorReset(useColor) << "] = " << colorHex(useColor)
                    << common::Format::word << inst << colorReset(useColor)
                    << "; " << isa::inst::disassemble(inst, hs().pc, useColor)
                    << std::endl;
            });
        }
    }

    if(program_args.get<bool>("--reg")) {
        hart.addRegisterReadListener(
            [this,
             useColor](std::string classname, uint64_t idx, uint64_t value) {
                *this->reg_log << "RD " << classname << "["
                               << colorAddr(useColor) << common::Format::dec
                               << idx << colorReset(useColor)
                               << "] = " << colorNew(useColor)
                               << common::Format::doubleword << (uint64_t)value
                               << colorReset(useColor) << std::endl;
            });
        hart.addRegisterWriteListener([this, useColor](
                                          std::string classname,
                                          uint64_t idx,
                                          uint64_t value,
                                          uint64_t oldvalue) {
            *this->reg_log << "WR " << classname << "[" << colorAddr(useColor)
                           << common::Format::dec << idx << colorReset(useColor)
                           << "] = " << colorNew(useColor)
                           << common::Format::doubleword << (uint64_t)value
                           << colorReset(useColor)
                           << "; OLD VALUE = " << colorOld(useColor)
                           << common::Format::doubleword << oldvalue
                           << colorReset(useColor) << std::endl;
        });
    }
    if(program_args.get<bool>("--mem")) {
        if(!program_args.get<bool>("--csv")) {
            hart.hs().mem().addAllocationListener(
                [this, useColor](uint64_t addr, uint64_t size) {
                    *this->mem_log << "ALLOCATE[" << colorAddr(useColor)
                                   << common::Format::doubleword << addr
                                   << colorReset(useColor) << " - "
                                   << colorAddr(useColor)
                                   << common::Format::doubleword << addr + size
                                   << colorReset(useColor) << "]" << std::endl;
                });

            hart.hs().mem().addReadListener(
                [this, useColor](uint64_t addr, uint64_t value, size_t size) {
                    *this->mem_log
                        << "RD MEM[" << colorAddr(useColor)
                        << common::Format::doubleword << addr
                        << colorReset(useColor) << "] = " << colorNew(useColor)
                        << common::Format::hexnum(size) << (uint64_t)value
                        << colorReset(useColor) << std::endl;
                });
            hart.hs().mem().addWriteListener([this, useColor](
                                                 uint64_t addr,
                                                 uint64_t value,
                                                 uint64_t oldvalue,
                                                 size_t size) {
                *this->mem_log
                    << "WR MEM[" << colorAddr(useColor)
                    << common::Format::doubleword << addr
                    << colorReset(useColor) << "] = " << colorNew(useColor)
                    << common::Format::hexnum(size) << (uint64_t)value
                    << colorReset(useColor)
                    << "; OLD VALUE = " << colorOld(useColor)
                    << common::Format::hexnum(size) << oldvalue
                    << colorReset(useColor) << std::endl;
            });
        } else {
            hart.hs().mem().addReadListener(
                [this](uint64_t addr, uint64_t value, size_t size) {
                    *this->mem_log << "rd-mem," << addr << ",";
                    if(size == 1) *this->mem_log << (uint8_t)value;
                    else if(size == 2) *this->mem_log << (uint16_t)value;
                    else if(size == 4) *this->mem_log << (uint32_t)value;
                    else *this->mem_log << value;
                    *this->mem_log << std::endl;
                });
            hart.hs().mem().addWriteListener(
                [this](
                    uint64_t addr,
                    uint64_t value,
                    [[maybe_unused]] uint64_t oldvalue,
                    size_t size) {
                    *this->mem_log << "wr-mem," << addr << ",";
                    // need extra casts so that cout doesn't tray and interpret
                    // as a char
                    if(size == 1) *this->mem_log << (uint64_t)(uint8_t)value;
                    else if(size == 2)
                        *this->mem_log << (uint64_t)(uint16_t)value;
                    else if(size == 4)
                        *this->mem_log << (uint64_t)(uint32_t)value;
                    else *this->mem_log << (uint64_t)value;
                    *this->mem_log << std::endl;
                });
        }
    }
}
void MainArguments::addControllerCallbacks(hart::Hart& hart) {
    bool useColor = program_args.get<bool>("--color");
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
                    [c](hart::HartState&) { c->doit(&std::cout); });
                break;
            case event::EventType::HART_BEFORE_EXECUTE:
                hart.addBeforeExecuteListener(
                    [c](hart::HartState&) { c->doit(&std::cout); });
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
        hart.addBeforeExecuteListener([w](hart::HartState&) { w->update(); });
        hart.addAfterExecuteListener([w](hart::HartState&) { w->update(); });
    }
}
std::vector<std::string> MainArguments::getArgV() { return simulated_argv; }
common::ordered_map<std::string, std::string> MainArguments::getEnvVars() {
    return simulated_env;
}

static std::unique_ptr<MainArguments> main_arg = nullptr;
MainArguments MainArguments::getMainArguments() {
    if(main_arg == nullptr) {
        main_arg = std::unique_ptr<MainArguments>(new MainArguments());
    }
    return *main_arg;
}

} // namespace arguments
