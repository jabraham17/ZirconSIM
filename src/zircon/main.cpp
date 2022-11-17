

#include "color/color.h"
#include "common/argparse.hpp"
#include "common/format.h"
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

int main(int argc, const char** argv) {

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
              "'mem')");

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

    program_args.add_argument("-S", "--stats")
        .default_value(false)
        .implicit_value(true)
        .help("dump runtime statistics");

    program_args.add_argument("-control")
        .nargs(2, 3)
        .append()
        .metavar("CONTROL")
        .help("a control sequence to apply");

    // everything after -- gets shoved into the remaining args
    std::vector<std::string> remaining_args;
    int newargc = argc;
    for(int i = 0; i < argc; i++) {
        if(argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == '\0') {
            newargc = i;
            for(i = i + 1; i < argc; i++) {
                remaining_args.push_back(argv[i]);
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

    std::string filename = program_args.get<std::string>("file");
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        std::cerr << "Failed to open '" << filename << "'" << std::endl;
        return 1;
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
    mem::MemoryImage memimg(0x800000);
    f.buildMemoryImage(memimg);
    auto start = f.getStartAddress();
    bool useColor = program_args.get<bool>("--color");
    cpu::Hart hart(memimg);

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
    auto colorReset = [useColor]() {
        return useColor ? color::getReset() : "";
    };

    if(program_args.get<bool>("--inst")) {
        hart.addBeforeExecuteListener(
            [inst_log, useColor, colorReset, colorHex, colorAddr](
                cpu::HartState& hs) {
                auto inst = hs.getInstWord();
                *inst_log << "PC[" << colorAddr() << common::Format::doubleword
                          << hs.pc << colorReset() << "] = " << colorHex()
                          << common::Format::word << inst << colorReset()
                          << "; "
                          << isa::inst::disassemble(inst, hs.pc, useColor)
                          << std::endl;
            });
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
            memimg.addAllocationListener(
                [mem_log, colorReset, colorAddr](uint64_t addr, uint64_t size) {
                    *mem_log << "ALLOCATE[" << colorAddr()
                             << common::Format::doubleword << addr
                             << colorReset() << " - " << colorAddr()
                             << common::Format::doubleword << addr + size
                             << colorReset() << "]" << std::endl;
                });

            memimg.addReadListener([mem_log, colorReset, colorAddr, colorNew](
                                       uint64_t addr,
                                       uint64_t value,
                                       size_t size) {
                *mem_log << "RD MEM[" << colorAddr()
                         << common::Format::doubleword << addr << colorReset()
                         << "] = " << colorNew() << common::Format::hexnum(size)
                         << (uint64_t)value << colorReset() << std::endl;
            });
            memimg.addWriteListener(
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
            memimg.addReadListener(
                [mem_log](uint64_t addr, uint64_t value, size_t size) {
                    *mem_log << "rd-mem," << addr << ",";
                    if(size == 1) *mem_log << (uint8_t)value;
                    else if(size == 2) *mem_log << (uint16_t)value;
                    else if(size == 4) *mem_log << (uint32_t)value;
                    else *mem_log << value;
                    *mem_log << std::endl;
                });
            memimg.addWriteListener([mem_log](
                                        uint64_t addr,
                                        uint64_t value,
                                        uint64_t oldvalue,
                                        size_t size) {
                    *mem_log << "wr-mem," << addr << ",";
                    if(size == 1) *mem_log << (uint8_t)value;
                    else if(size == 2) *mem_log << (uint16_t)value;
                    else if(size == 4) *mem_log << (uint32_t)value;
                    else *mem_log << value;
                    *mem_log << std::endl;
            });
        }
    }
    Stats stats;
    if(program_args.get<bool>("--stats")) {
        hart.addBeforeExecuteListener(
            [&stats](cpu::HartState& hs) { stats.count(hs); });
    }

    // add commandline args
    auto control_args = program_args.get<std::vector<std::string>>("-control");
    if(!control_args.empty()) {
        auto parser = controller::parser::Parser(control_args);
        auto parsed_commands = parser.parse();
        for(auto a : parsed_commands.allActions()) {
            a->hs = &hart.hs;
        }
        for(auto c : parsed_commands.allConditions()) {
            c->hs = &hart.hs;
        }
        for(auto c : parsed_commands.commands) {
            switch(c->getEventType()) {
                case event::EventType::HART_AFTER_EXECUTE:
                    hart.addAfterExecuteListener(
                        [c](cpu::HartState& hs) { c->doit(&std::cout); });
                    break;
                case event::EventType::HART_BEFORE_EXECUTE:
                    hart.addBeforeExecuteListener(
                        [c](cpu::HartState& hs) { c->doit(&std::cout); });
                    break;
                case event::EventType::MEM_READ:
                    hart.hs.memimg.addReadListener(
                        [c](uint64_t, uint64_t, size_t) {
                            c->doit(&std::cout);
                        });
                    break;
                case event::EventType::MEM_WRITE:
                    hart.hs.memimg.addWriteListener(
                        [c](uint64_t, uint64_t, uint64_t, size_t) {
                            c->doit(&std::cout);
                        });
                    break;
                case event::EventType::MEM_ALLOCATION:
                    hart.hs.memimg.addAllocationListener(
                        [c](uint64_t, uint64_t) { c->doit(&std::cout); });
                    break;
                case event::EventType::REG_READ:
                    hart.hs.rf.addReadListener(
                        [c](std::string, uint64_t, uint64_t) {
                            c->doit(&std::cout);
                        });
                    break;
                case event::EventType::REG_WRITE:
                    hart.hs.rf.addWriteListener(
                        [c](std::string, uint64_t, uint64_t, uint64_t) {
                            c->doit(&std::cout);
                        });
                    break;
                default: std::cerr << "No Event Handler Defined\n";
            }
        }
    }

    hart.init();
    hart.execute(start);

    if(program_args.get<bool>("--stats")) {
        std::cout << stats.dump() << std::endl;
    }

    return 0;
}
