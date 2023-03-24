

#include "arguments.h"

#include "elf/elf.h"
#include "hart/hart.h"
#include "ishell/parser/parser.h"
#include "ishell/repl.h"
#include "trace/stats.h"

int main(int argc, const char** argv, const char** envp) {

    auto args = arguments::MainArguments::getMainArguments();
    try {
        args.parse(argc, argv, envp);
    } catch(const arguments::ArgumentException& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    elf::File elf(args.getInputFile());
    hart::Hart hart(std::make_shared<mem::MemoryImage>());
    elf.buildMemoryImage(hart.hs().mem());
    auto start = elf.getStartAddress();
    hart.hs().setElfSymbols(elf.getSymbolToAddressMap());

    args.addCallbacks(hart, elf);
    args.addControllerCallbacks(hart);

    Stats stats;
    if(args.accessRawArguments().get<bool>("--stats")) {
        hart.addBeforeExecuteListener(
            [&stats](hart::HartState& hs) { stats.count(hs); });
    }

    hart.init(args.getArgV(), args.getEnvVars());
    hart.hs().setPC(start);

    ishell::Repl repl(&hart.hs());

    if(args.accessRawArguments().get<bool>("--start-paused")) {
        hart.hs().pause();
    } else {
        hart.hs().start();
    }
    hart.startExecution();
    repl.run();

    hart.wait_till_done();
    repl.wait_till_done();

    if(args.accessRawArguments().get<bool>("--stats")) {
        std::cout << stats.dump() << std::endl;
    }

    return 0;
}
