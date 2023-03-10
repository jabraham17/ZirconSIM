#include "repl.h"

#include "command/command.h"
#include "hart/hartstate.h"
#include "ishell/parser/parser.h"

#include <unistd.h>
#include <termios.h>
#include "getline.h"

namespace ishell {

Repl::Repl(hart::HartState* hs) : hs(hs), sync_point(), execution_thread(&Repl::execute, this) {}

void Repl::run() {
    sync_point.signal();
}
void Repl::wait_till_done() {
    sync_point.wait();
        execution_thread.join();
    }

void Repl::execute() {
    sync_point.wait();
    common::debug::logln("Starting Repl::execute()");
    auto parser = ishell::parser::Parser();
    auto lineInput = getline();
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
        }
        else if(hs->isRunning()) {
            // keep running
            std::this_thread::yield();
        }
        else {
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
