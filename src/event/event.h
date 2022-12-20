#ifndef ZIRCON_EVENT_EVENT_H_
#define ZIRCON_EVENT_EVENT_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace event {

// namespace internal {
//     void addEvent(std::any p);
//     void removeEvent(std::any p);
// }

class EventInterface {

  public:
};

template <typename... Types> class Event : public EventInterface {
  public:
    using callback_type = std::function<void(Types...)>;

  private:
    std::vector<callback_type> callbacks;

  public:
    // void Event() {
    //     addEvent(this);
    // }
    // void ~Event() {
    //     removeEvent(this);
    //     delete callbacks;
    // }
    void operator()(Types... args) { call(args...); }
    void call(Types... args) {
        for(auto c : callbacks) {
            c(args...);
        }
    }
    void addListener(callback_type c) { callbacks.push_back(c); }
};

// class EventInterface {
//     private:
//      EventType et;

//     public:
//     template <typename... Types> Event<Types...>* to() {

//     }

// };

enum class EventSubsystemType {
#define EVENT_SUBSYSTEM(e) e,
#include "events.inc"
    NONE,
};
bool isEventSubsystemType(std::string s);

EventSubsystemType getEventSubsystemType(std::string s);

enum class EventType: uint64_t {
#define EVENT_NAME(sub, e) sub##_##e,
#include "events.inc"
    NONE,
};
bool isEventType(std::string s);
EventType getEventType(std::string s);
// class EventType {
//     using ValueType = int;
//     ValueType value_;
//     static const ValueType NONE = 0;
//     #define EVENT_NAME(subsystem_name, event_name) \
//     static const ValueType subsystem_name##_##event_name;

//     TraceMode(ValueType v) : _value(v) {}
//     TraceMode() : _value(NONE) {}

//     TraceMode& operator=(const ValueType& v) {
//         this->_value = v;
//         return *this;
//     }
//     operator ValueType() const { return this->_value; }
// };

// #define EVENT_TYPES(V) \
//     V(INST_AFTER_EXECUTE,"inst:after_execute")

// enum class EventType {
//         #define EVENT_CASE(ev,ev_str) ev,
//     EVENT_TYPES(EVENT_CASE)
//         #undef EVENT_CASE
//         NONE
// };
// static std::string getEventName(EventType t) {
//     #define EVENT_IF(ev,ev_str) if(t == EventType::ev) return
//     std::string(ev_str); EVENT_TYPES(EVENT_IF) #undef EVENT_IF return "";
// }
// static EventType parseEventName(std::string s) {
//     #define EVENT_IF(ev,ev_str) if(s == ev_str) return EventType::ev;
//     EVENT_TYPES(EVENT_IF)
//     #undef EVENT_IF
//     return EventType::NONE;
// }

// #define ACTION_TYPES(V) \
//     V(STOP,"stop") \
//     V(DUMP_REGS,"dump_regs")

// enum class ActionType {
//         #define ACTION_CASE(ev,ev_str) ev,
//     ACTION_TYPES(ACTION_CASE)
//         #undef ACTION_CASE
//         NONE
// };
// static std::string getActionName(ActionType t) {
//     #define ACTION_IF(ac,ac_str) if(t == ActionType::ac) return
//     std::string(ac_str); ACTION_TYPES(ACTION_IF) #undef ACTION_IF return "";
// }
// static ActionType parseActionName(std::string s) {
//     #define ACTION_IF(ac,ac_str) if(s == ac_str) return ActionType::ac;
//     ACTION_TYPES(ACTION_IF)
//     #undef ACTION_IF
//     return ActionType::NONE;
// }

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
