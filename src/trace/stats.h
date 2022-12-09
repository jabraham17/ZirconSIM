
#ifndef ZIRCON_TRACE_STATS_H_
#define ZIRCON_TRACE_STATS_H_

#include "hart/hartstate.h"

#include <map>
#include <string>

namespace cpu {
class HartState;
}

class Stats {
  private:
    std::map<std::string, float> counters;
    std::map<std::string, float> computed_counters;

  public:
    Stats();
    void count(const hart::HartState&);

    std::string dump();
};

#endif
