#ifndef ZIRCON_EVENT_EVENT_H_
#define ZIRCON_EVENT_EVENT_H_

// #include "cpu/cpu.h"
#include <algorithm>
#include <functional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace event {
template <typename... Types> class Event {
  public:
    using callback_type = std::function<void(Types...)>;

  private:
    std::vector<callback_type> callbacks;

  public:
    void operator()(Types... args) { call(args...); }
    void call(Types... args) {
        for(auto c : callbacks) {
            c(args...);
        }
    }
    void addListener(callback_type c) { callbacks.push_back(c); }
};

//  memory access
//         memory read
//         memory write
//         memory read byte
//         memory write byte
//         memory read half
//         memory write half
//         memory read word
//         memory write word
//         memory read doubleword
//         memory write doubleword
//         memory exception
//         memory allocation
//         register access
//         register read
//         register write
//         instruction execute before
//         instruction execute after

} // namespace event

#endif
