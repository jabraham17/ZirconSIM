

#include "arguments.h"

#include "elf/elf.h"
#include "hart/hart.h"
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

    args.addCallbacks(hart, elf);
    args.addControllerCallbacks(hart);

    Stats stats;
    if(args.accessRawArguments().get<bool>("--stats")) {
        hart.addBeforeExecuteListener(
            [&stats](hart::HartState& hs) { stats.count(hs); });
    }

    auto t = std::thread([&hart]() {
        while(1) {
            hart.hs().waitForExecutionStateChange();
            if(hart.hs().isPaused()) {
                std::cout.flush();
                std::cout << "> ";
                std::string input;
                std::cin >> input;
                if(input == "stop") {
                    hart.hs().stop();
                    break;
                } else if(input == "resume") hart.hs().resume();
            } else if(
                hart.hs().getExecutionState() ==
                    hart::ExecutionState::STOPPED ||
                hart.hs().getExecutionState() ==
                    hart::ExecutionState::INVALID_STATE) {
                break;
            }
        }
    });

    hart.init(args.getArgV(), args.getEnvVars());
    hart.hs().start(start);
    hart.wait_till_done();
    t.join();

    if(args.accessRawArguments().get<bool>("--stats")) {
        std::cout << stats.dump() << std::endl;
    }

    return 0;
}
