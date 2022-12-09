

#include "arguments.h"

#include "elf/elf.h"
#include "hart/hart.h"
#include "trace/stats.h"

int main(int argc, const char** argv, const char** envp) {

    auto args = arguments::MainArguments::getMainArguments();
    args.parse(argc, argv, envp);

    elf::File elf(std::move(args.getInputFile()));
    hart::Hart hart(std::make_shared<mem::MemoryImage>());
    elf.buildMemoryImage(hart.hs().mem());
    auto start = elf.getStartAddress();

    args.addCallbacks(hart, elf);
    args.addControllerCallbacks(hart);

    Stats stats;
    if(args.accessRawArguments().get<bool>("--stats")) {
        hart.addBeforeExecuteListener(
            [&stats](hart::HartState& hs) { stats.count(hs); });
    }

    hart.init(args.getArgV(), args.getEnvVars());
    hart.execute(start);

    if(args.accessRawArguments().get<bool>("--stats")) {
        std::cout << stats.dump() << std::endl;
    }

    return 0;
}
