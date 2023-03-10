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
    while(1) {
        common::debug::logln("In Repl::execute()");
        if(hs->isPaused()) {
            std::cout.flush();
            std::cout << "> ";
            std::cout.flush();
            auto input = getNextLine();
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

// int getch()
// {
//  int ch;
//  struct termios oldt;
//  struct termios newt;
//  tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
//  newt = oldt; /* copy old settings to new settings */
//  newt.c_lflag &= ~(ICANON | ECHO  | ECHOE); /* make one change to old settings in new settings */
//  tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediatly */
//  ch = getchar(); /* standard getchar call */
//  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */
//  return ch; /*return received char */
// }

// void goLeft(int n = 1) {
//     std::cout << "\033[1D";
// }
// void goRight(int n = 1);
// void goUp(int n=1);
// void goDown(int n=1);

// void setCursor(std::pair<int,int> p) {
//     std::cout << "\033[" << p.first << ";" << p.second << "H";
// }

// std::pair<int, int> getCursor() {
//     std::cout << "\033[6n";
//     int c = getch();
//     if(c == '\033') {
//         c = getch();
//         if(c == '[') {
//             // next chars are all n
//             int n, m;
//             char buf[10];
//             char* ptr = buf;
//             do {
//                 *ptr = (char)getch();
//                 ptr++;
//             } while(*(ptr-1) != ';');
//             *(ptr-1) = 0;
//             n = std::stol(std::string(buf));

//             ptr = buf;
//             do {
//                 *ptr = (char)getch();
//                 ptr++;
//             } while(*(ptr-1) != 'R');
//             *(ptr-1) = 0;
//             m = std::stol(std::string(buf));
//             return {n,m};
//         }
//     }
//     return {0,0};
// }

// std::pair<int, int> getSize() {
//     auto cur = getCursor();
//     setCursor({999,999});
//     auto size = getCursor();
//     setCursor(cur);
//     return size;
// }

// void backspace() {

// }

std::string Repl::getNextLine() {
    return getline::getline();
}

// std::string Repl::getNextLine() {
//     std::vector<char> line_chars;
//     int c;
//     auto size = getSize();
//     std::cerr << size.first << ","<< size.second << "\n";
//     while( (c= getch()) != 0) {
//         if(c == 10) {
//             std::cout << std::endl;
//             break;
//         }
//         else if(c == 27) { // \033
//             if(getch() == 91) { // [
//                 if(getch() == 68) goLeft();
//             }
//         }
//             else if (c == 0x7f) {
//             // go one char left
//             printf("\b");
//             // overwrite the char with whitespace
//             printf(" ");
//             // go back to "now removed char position"
//             printf("\b");
//         }
//         else {
//             std::cout << (char)c;
//             line_chars.push_back(c);
//         }
//         auto p = getCursor();
//         std::cerr << p.first << ", " << p.second;

//     }
//     // while(std::cin.get(c)) {
//     //     if(c == '\n') break;
//     //     else if(c == 'l') {
//     //         // get next char
//     //         // std::cin.get(c);
//     //         // if(c == 75) 
//     //         std::cout << "\033[;2D";
//     //     }
//     //     else {
//     //         line_chars.push_back(c);
//     //     }

//     // }
//     return std::string(line_chars.begin(), line_chars.end());
// }

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
