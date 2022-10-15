

#include "cpu/cpu.h"
#include "elf/elf.h"
#include "mem/memory-image.h"
#include "trace/trace.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <popt.h>

struct zircon_args {
    char* file;
    char* logfile; // unimplemented
    TraceMode traces;
    bool separate_files; // unimplemented
    int stats;

    zircon_args()
        : file(strdup("a.out")), logfile(strdup("log.txt")),
          traces(TraceMode::NONE), separate_files(false), stats(false) {}
};

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
        {"output",
         'o',
         POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
         &args.logfile,
         0,
         "output log file",
         "OUTPUT"},
        {"trace-inst",
         'I',
         POPT_ARG_VAL | POPT_ARGFLAG_OR,
         &args.traces,
         TraceMode::INSTRUCTION,
         "trace instructions",
         "TRACE"},
        {"trace-mem",
         'M',
         POPT_ARG_VAL | POPT_ARGFLAG_OR,
         &args.traces,
         TraceMode::MEMORY,
         "trace memory",
         "TRACE"},
        {"trace-reg",
         'R',
         POPT_ARG_VAL | POPT_ARGFLAG_OR,
         &args.traces,
         TraceMode::REGISTER,
         "trace registers",
         "TRACE"},
        {"stats", 'S', POPT_ARG_NONE, &args.stats, 0, "log statistics", 0},
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

    elf::File f(std::move(is));
    mem::MemoryImage memimg(0x80000, args.traces);
    f.buildMemoryImage(memimg);
    auto start = f.getStartAddress();

    cpu::Hart hart(memimg, args.traces, args.stats);
    hart.init();
    hart.execute(start);

    return 0;
}
