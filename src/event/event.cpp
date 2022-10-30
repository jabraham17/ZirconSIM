
#include "event.h"
#include <unordered_map>

namespace event {

namespace callback {
Callback::Callback(callback_t func) : func(func) {}
void Callback::operator()(state hs) { call(hs); }
void Callback::call(state hs) {
    if(func) func(hs);
}

Logger::Logger(log stream, logger_t logger)
    : Callback(nullptr), stream(stream), logger(logger) {}
Logger::~Logger() { stream.flush(); };
void Logger::call(state hs) {
    if(logger) logger(stream, hs);
}
} // namespace callback

void Event::registerCallback(std::unique_ptr<callback::Callback> c) {
    callbacks.push_back(std::move(c));
}
void Event::registerCallback(callback::callback_t c) {
    registerCallback(std::make_unique<callback::Callback>(c));
}
void Event::registerCallback(log s, callback::logger_t c) {
    registerCallback(std::make_unique<callback::Logger>(s, c));
}
void Event::operator()(state hs) { call(hs); }
void Event::call(state hs) {
    std::for_each(callbacks.begin(), callbacks.end(), [&hs](auto& c) {
        (*c)(hs);
    });
}

using EventList = std::unordered_map<std::string, Event>;
// use anon struct so we can use the constructor to build a map
// forced to do this because we cant use the map constructor 
// because Event is not copy constructible because unique ptr
struct {
    EventList list{[]() {
        EventList l;
        l.emplace("memory access", Event());
        l.emplace("memory access", Event());
        l.emplace("memory read", Event());
        l.emplace("memory write", Event());
        l.emplace("memory exception", Event());
        l.emplace("memory allocation", Event());
        l.emplace("register access", Event());
        l.emplace("register read", Event());
        l.emplace("register write", Event());
        l.emplace("instruction execute before", Event());
        l.emplace("instruction execute after", Event());
        return l;
    }()};
} event_list;
Event undefined_event;

Event& getEvent(const std::string& name) {
    auto it = event_list.list.find(name);
    if(it != event_list.list.end()) return it->second;
    else return undefined_event;
}

// callback list
/*
memory
- access
- read
- write
- exception
- allocation

register
- access
- read
- write

instruction
- before execute
- after execute

*/

} // namespace event
