

#include "arguments.h"

#include "elf/elf.h"
#include "hart/hart.h"
#include "trace/stats.h"
#include "ishell/parser/parser.h"

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
        auto parser = ishell::parser::Parser();
        while(1) {
            // hart.hs().waitForExecutionStateChange();
            if(hart.hs().isPaused()) {
                std::cout.flush();
                std::cout << "> ";
                std::string input;
                std::getline(std::cin, input);

                try {
                auto control = parser.parse(input);
                control->setHS(&hart.hs());
                if(auto command = std::dynamic_pointer_cast<command::Command>(control)) {
                    command->doit(&std::cout);
                }
                else {
                    std::cerr << "Only COMMANDs are supported at this time\n";
                }
                }
                catch(const ishell::parser::ParseException& pe) {
                    std::cerr << "Invalid command\n";
                }

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
    // TODO: very temporary, race condition
    if(args.accessRawArguments().get<bool>("--start-paused")) {
        hart.hs().pause();
    }
    hart.wait_till_done();
    t.join();

    if(args.accessRawArguments().get<bool>("--stats")) {
        std::cout << stats.dump() << std::endl;
    }

    return 0;
}
