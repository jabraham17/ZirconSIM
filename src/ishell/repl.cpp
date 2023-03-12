#include "repl.h"

#include "getline.h"

#include "command/command.h"
#include "common/utils.h"
#include "hart/hartstate.h"
#include "hart/isa/rf.h"
#include "ishell/parser/parser.h"

#include <termios.h>
#include <unistd.h>

namespace ishell {

Repl::Repl(hart::HartState* hs)
    : hs(hs), sync_point(), execution_thread(&Repl::execute, this) {}

void Repl::run() { sync_point.signal(); }
void Repl::wait_till_done() {
    sync_point.wait();
    execution_thread.join();
}

void Repl::execute() {
    sync_point.wait();
    common::debug::logln("Starting Repl::execute()");
    auto parser = ishell::parser::Parser();
    auto lineInput = getline();
    // add keyword completion
    lineInput.addCompletetion([](auto s) {
        // split s by space, we do completions on most up to date word
        auto elms = common::utils::split(s, " ");
        // we do matching case insensitive
        const auto lastElm = common::utils::toupper(elms.back());

        std::vector<std::string> suggest;
        // loop through all keywords, if it starts with last elm suggest it
        for(auto k : ishell::parser::TokenType::validKeywords()) {
            if(common::utils::startsWith(k, lastElm)) {
                // suggest string is previous string with replacement for last
                // elm
                std::string sug;
                if(elms.size() >= 2) {
                    sug =
                        common::utils::join(elms.begin(), elms.end() - 1, " ") +
                        " " + k;
                } else {
                    sug = k;
                }
                suggest.push_back(sug);
            }
        }

        return suggest;
    });
    // add pc and mem completion
    lineInput.addCompletetion([](auto s) {
        // split s by space, we do completions on most up to date word
        auto elms = common::utils::split(s, " ");
        // we do matching case insensitive
        const auto lastElm = common::utils::toupper(elms.back());

        std::vector<std::string> suggest;
        if(lastElm.size() >= 1 && lastElm[0] == '$') {
            std::string justTheName = lastElm.substr(1);
            if(common::utils::startsWith("M", justTheName)) {
                std::string sug;
                if(elms.size() >= 2) {
                    sug =
                        common::utils::join(elms.begin(), elms.end() - 1, " ") +
                        " $M";
                } else {
                    sug = "$M";
                }
                suggest.push_back(sug);
            }
            if(common::utils::startsWith("PC", justTheName)) {
                std::string sug;
                if(elms.size() >= 2) {
                    sug =
                        common::utils::join(elms.begin(), elms.end() - 1, " ") +
                        " $PC";
                } else {
                    sug = "$PC";
                }
                suggest.push_back(sug);
            }
        }
        return suggest;
    });
    // add register completion
    lineInput.addCompletetion([](auto s) {
        // split s by space, we do completions on most up to date word
        auto elms = common::utils::split(s, " ");
        const auto lastElm = elms.back();

        std::vector<std::string> suggest;
        if(lastElm.size() >= 1 && lastElm[0] == '$') {
            std::string justTheName = lastElm.substr(1);
            // loop through all registers, if it starts with last elm suggest it
            for(auto r : isa::rf::getAllPossibleRegisterNames()) {
                if(common::utils::startsWith(r, justTheName)) {
                    // suggest string is previous string with replacement for
                    // last elm
                    std::string sug;
                    if(elms.size() >= 2) {
                        sug = common::utils::join(
                                  elms.begin(),
                                  elms.end() - 1,
                                  " ") +
                              " $" + r;
                    } else {
                        sug = "$" + r;
                    }
                    suggest.push_back(sug);
                }
            }
        }

        return suggest;
    });

    while(1) {
        common::debug::logln("In Repl::execute()");
        if(hs->isPaused()) {
            std::string input;
            auto res = lineInput.get(input, ">>> ");
            if(!res) {
                // stop the hart on err
                hs->stop();
                continue;
            }
            try {
                auto control = parser.parse(input);
                control->setHS(hs);
                if(auto command =
                       std::dynamic_pointer_cast<command::Command>(control)) {
                    command->doit(&std::cout);
                } else {
                    std::cerr << "Only COMMANDs are supported at this time\n";
                }
            } catch(const ishell::parser::ParseException& pe) {
                std::cerr << "Invalid command\n";
            }
        } else if(hs->isRunning()) {
            // keep running
            std::this_thread::yield();
        } else {
            break;
        }
    }
    common::debug::logln("Finished with Repl::execute()");
    sync_point.signal();
}

} // namespace ishell

/*

behavior of repl main thread

it should immediately start running and watch the state of the hart
when the hart enters a paused state, the repl should wake up
  it shoul;d listen for commands
    on command being entered (entered key is pressed or EOF)
    execute command
    if the result of the command is a start/resume state, continue
    if the result of the command is a stop state, exit the simulator
    if the result of the command is a pause state, listen for more commands


*/
