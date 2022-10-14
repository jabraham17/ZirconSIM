
#ifndef ZIRCON_TRACE_STATS_H_
#define ZIRCON_TRACE_STATS_H_

#include <map>
#include <string>

namespace cpu {
class HartState;
}

class Stats {
  private:
    std::map<std::string, float> counters;
    std::map<std::string, float> computed_counters;
    bool enabled;

  public:
    Stats();
    void enable() { enabled = true; }
    void disable() { enabled = false; }
    void setState(bool s) { enabled = s; }
    bool isEnabled() { return enabled; }

    void count(const cpu::HartState&);

    std::string dump();
};

#endif
