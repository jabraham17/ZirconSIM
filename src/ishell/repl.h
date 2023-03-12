#ifndef ZIRCON_ISHELL_REPL_H_
#define ZIRCON_ISHELL_REPL_H_

#include "common/threading/syncpoint.h"

#include <string>
#include <thread>

namespace hart {
class HartState;
}

namespace ishell {

class Repl {
  private:
    hart::HartState* hs;
    common::threading::syncpoint sync_point;
    std::thread execution_thread;

  public:
    Repl(hart::HartState* hs);
    void run();
    void wait_till_done();

  private:
    // main execution function for thread
    void execute();
    // void handleLine(const std::string& );
    std::string getNextLine();
};

} // namespace ishell

#endif
