#ifndef ZIRCON_EVENT_EVENT_H_
#define ZIRCON_EVENT_EVENT_H_

#include "cpu/cpu.h"
#include <algorithm>
#include <functional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

//

namespace event {
using state = cpu::HartState&;
using log = std::ostream&;

namespace callback {
using callback_t = void (*)(state);
using logger_t = void (*)(log, state);

class Callback {
  private:
    callback_t func;

  public:
    Callback(callback_t func);
    virtual ~Callback() = default;

    void operator()(state hs);
    virtual void call(state hs);
};
class Logger : public Callback {
  private:
    log stream;
    logger_t logger;

  public:
    Logger(log stream, logger_t logger);
    virtual ~Logger();
    virtual void call(state hs) override;
};

} // namespace callback

class Event {
  private:
    std::vector<std::unique_ptr<callback::Callback>> callbacks;

  public:
    Event() = default;
    void registerCallback(std::unique_ptr<callback::Callback> c);
    void registerCallback(callback::callback_t c);
    void registerCallback(log s, callback::logger_t c);
    void operator()(state hs);
    void call(state hs);
};

Event& getEvent(const std::string& name);

} // namespace event

#endif
