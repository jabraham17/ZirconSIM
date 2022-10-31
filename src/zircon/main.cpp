

#include "common/format.h"
#include "cpu/cpu.h"
#include "cpu/isa/inst.h"
#include "elf/elf.h"
#include "event/event.h"
#include "mem/memory-image.h"
#include "trace/stats.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <popt.h>
#include <unordered_map>

struct zircon_args {
    char* file;
    char* logfile;
    int trace_inst;
    char* trace_inst_file;
    int trace_mem;
    char* trace_mem_file;
    int trace_reg;
    char* trace_reg_file;
    int stats;
    int color;

    zircon_args()
        : file(strdup("a.out")), logfile(nullptr), trace_inst(false),
          trace_inst_file(nullptr), trace_mem(false), trace_mem_file(nullptr),
          trace_reg(false), trace_reg_file(nullptr), stats(false),
          color(false) {}
};

// FIXME: this function relies upon the fnames for different files being the
// same, this is not guaranteed to be the same.
// CONSIDER: ./file.txt and ././file.txt and ../dir/file.txt could all be the
// same file
auto getFileStreamIfTrue(bool cond, char* fname, std::ostream& alternative) {
    static std::unordered_map<std::string, std::ostream*> file_buffer;
    auto inst_log = (&alternative);
    if(cond && fname) {
        auto it = file_buffer.find(fname);
        if(it != file_buffer.end()) {
            // return old file handle
            inst_log = it->second;
        } else {
            // open new file handle
            auto handle = (new std::ofstream(fname));
            file_buffer[fname] = handle;
            inst_log = handle;
        }
    }
    return inst_log;
}

int main(int argc, const char** argv) {
    // https://linux.die.net/man/3/popt
    //  struct poptOption {
    //    const char* longName;
    //    char shortName;
    //    int argInfo;
    //    void* arg;
    //    int val;
    //    char* descrip;
    //    char* argDescrip;
    //  };
    zircon_args args;
    struct poptOption options[] = {
        {"file",
         'f',
         POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.file,
         0,
         "elf64 file execute read from",
         "FILE"},
        {"color",
         'c',
         POPT_ARG_NONE | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.color,
         0,
         "colorize output",
         0},
        {"output",
         'o',
         POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.logfile,
         0,
         "output log file",
         "OUTPUT"},
        {"trace-inst",
         'I',
         POPT_ARG_NONE | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.trace_inst,
         0,
         "trace instructions",
         0},
        {"trace-inst-file",
         0,
         POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.trace_inst_file,
         0,
         "instructions log file",
         0},
        {"trace-mem",
         'M',
         POPT_ARG_NONE | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.trace_mem,
         0,
         "trace memory",
         0},
        {"trace-mem-file",
         0,
         POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.trace_mem_file,
         0,
         "memory log file",
         0},
        {"trace-reg",
         'R',
         POPT_ARG_NONE | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.trace_reg,
         0,
         "trace registers",
         0},
        {"trace-reg-file",
         0,
         POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.trace_reg_file,
         0,
         "register log file",
         0},
        {"stats",
         'S',
         POPT_ARG_NONE | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.stats,
         0,
         "log statistics",
         0},
        POPT_AUTOHELP POPT_TABLEEND};

    poptContext optCon = poptGetContext(argv[0], argc, argv, options, 0);
    char rc;
    while((rc = poptGetNextOpt(optCon)) >= 0) {
    }
    if(rc < -1) {
        fprintf(
            stderr,
            "Option Error on '%s': %s\n",
            poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
            poptStrerror(rc));
        return 1;
    }

    std::string filename = args.file;
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        std::cerr << "Failed to open '" << filename << "'" << std::endl;
        return 1;
    }

    auto inst_log = getFileStreamIfTrue(
        args.trace_inst && args.trace_inst_file,
        args.trace_inst_file,
        std::cout);
    auto mem_log = getFileStreamIfTrue(
        args.trace_mem && args.trace_mem_file,
        args.trace_reg_file,
        std::cout);
    auto reg_log = getFileStreamIfTrue(
        args.trace_reg && args.trace_reg_file,
        args.trace_reg_file,
        std::cout);

    elf::File f(std::move(is));
    mem::MemoryImage memimg(0x800000);
    f.buildMemoryImage(memimg);
    auto start = f.getStartAddress();

    if(args.trace_mem) {
        memimg.addAllocationListener([mem_log](uint64_t addr, uint64_t size) {
            *mem_log << "ALLOCATE[" << common::Format::doubleword << addr
                     << " - " << common::Format::doubleword << addr + size
                     << "]" << std::endl;
        });

        memimg.addReadListener(
            [mem_log](uint64_t addr, uint64_t value, size_t size) {
                *mem_log << "RD MEM[" << common::Format::doubleword << addr
                         << "] = " << common::Format::hexnum(size)
                         << (uint64_t)value << std::endl;
            });
        memimg.addWriteListener([mem_log](
                                    uint64_t addr,
                                    uint64_t value,
                                    uint64_t oldvalue,
                                    size_t size) {
            *mem_log << "WR MEM[" << common::Format::doubleword << addr
                     << "] = " << common::Format::hexnum(size)
                     << (uint64_t)value
                     << "; OLD VALUE = " << common::Format::hexnum(size)
                     << oldvalue << std::endl;
        });
    }

    cpu::Hart hart(memimg);

    if(args.trace_inst) {
        hart.addBeforeExecuteListener([inst_log, args](cpu::HartState& hs) {
            auto inst = hs.getInstWord();
            *inst_log << "PC[" << common::Format::doubleword << hs.pc
                      << "] = " << common::Format::word << inst << "; "
                      << isa::inst::disassemble(inst, hs.pc, args.color)
                      << std::endl;
        });
    }

    if(args.trace_reg) {
        hart.addRegisterReadListener(
            [reg_log](std::string classname, uint64_t idx, uint64_t value) {
                *reg_log << "RD " << classname << "[" << common::Format::dec
                         << idx << "] = " << common::Format::doubleword
                         << (uint64_t)value << std::endl;
            });
        hart.addRegisterWriteListener([reg_log](
                                          std::string classname,
                                          uint64_t idx,
                                          uint64_t value,
                                          uint64_t oldvalue) {
            *reg_log << "WR " << classname << "[" << common::Format::dec << idx
                     << "] = " << common::Format::doubleword << (uint64_t)value
                     << "; OLD VALUE = " << common::Format::doubleword
                     << oldvalue << std::endl;
        });
    }

    Stats stats;
    if(args.stats) {
        hart.addBeforeExecuteListener(
            [&stats](cpu::HartState& hs) { stats.count(hs); });
    }

    hart.init();
    hart.execute(start);

    if(args.stats) {
        std::cout << stats.dump() << std::endl;
    }

    return 0;
}
