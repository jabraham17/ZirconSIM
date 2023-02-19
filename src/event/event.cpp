
#include "event.h"

// #include <any>

namespace event {

// namespace internal {
// std::vector<std::any> events;
//     void addEvent(std::any p) {
//         events.push_back(p);
//     }
//     void removeEvent(std::any p);
// }

bool isEventSubsystemType(std::string s) {
#define EVENT_SUBSYSTEM(e)                                                     \
    if(s == #e) return true;
#include "events.inc"
    return false;
}
EventSubsystemType getEventSubsystemType(std::string s) {
#define EVENT_SUBSYSTEM(e)                                                     \
    if(s == #e) return EventSubsystemType::e;
#include "events.inc"
    return EventSubsystemType::NONE;
}

std::string getEventName(EventType et) {
#define EVENT_NAME(sub, e)                                                     \
    if(et == EventType::sub##_##e) return #sub "_" #e;
#include "events.inc"
    return "";
}
std::vector<EventType> getDefaultEventTypes() {
    return {EventType::HART_BEFORE_EXECUTE};
}

bool isEventType(std::string s) {
#define EVENT_NAME(sub, e)                                                     \
    if(s == #sub "_" #e || s == #e) return true;
#include "events.inc"
    return false;
}
EventType getEventType(std::string s) {
#define EVENT_NAME(sub, e)                                                     \
    if(s == #sub "_" #e) return EventType::sub##_##e;
#include "events.inc"
    return EventType::NONE;
}

} // namespace event
