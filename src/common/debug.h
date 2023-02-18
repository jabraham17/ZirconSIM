#ifndef ZIRCON_COMMON_DEBUG_H_
#define ZIRCON_COMMON_DEBUG_H_

#include <iostream>
#include <string>

#define DEBUG_CATAGORIES(F)                                                    \
    F(NONE, 0x0)                                                               \
    F(GENERAL, 0x1)                                                            \
    F(PARSER, 0x2)                                                             \
    F(ELF, 0x4)                                                                \
    F(SYSCALL, 0x8)

namespace common {
namespace debug {

struct DebugType {
    using ValueType = int;

  private:
    ValueType value_;

  public:
#define TOKEN(d, v) static const ValueType d = v;
    DEBUG_CATAGORIES(TOKEN)
#undef TOKEN

    DebugType(ValueType v = NONE) : value_(v) {}
    DebugType(std::string s);
    operator ValueType() { return value_; }
    operator ValueType() const { return value_; }
    explicit operator std::string() const {
#define DEBUG_CASE(d, v)                                                       \
    if(this->value_ == d) return #d;
        DEBUG_CATAGORIES(DEBUG_CASE)
#undef DEBUG_CASE
        return "";
    }
    explicit operator std::string() {
        return std::string(*const_cast<const DebugType*>(this));
    }
    DebugType operator=(ValueType v) {
        this->value_ = v;
        return *this;
    }
    bool operator==(const DebugType& other) const {
        return this->value_ == other.value_;
    }
    bool operator==(const DebugType& other) {
        return *const_cast<const DebugType*>(this) == other;
    }
    bool operator==(const ValueType& value) const {
        return this->value_ == value;
    }
    bool operator==(const ValueType& value) {
        return *const_cast<const DebugType*>(this) == value;
    }
    DebugType operator|(const DebugType& other) const {
        return DebugType(this->value_ | other.value_);
    }
    DebugType operator|(const DebugType& other) {
        return *const_cast<const DebugType*>(this) | other;
    }
    void operator|=(const DebugType& other) { this->value_ |= other.value_; }

    DebugType operator&(const DebugType& other) const {
        return DebugType(this->value_ & other.value_);
    }
    DebugType operator&(const DebugType& other) {
        return *const_cast<const DebugType*>(this) & other;
    }
    void operator&=(const DebugType& other) { this->value_ &= other.value_; }
};

bool checkDebugState(DebugType dt);
void setDebugState(DebugType dt);
void updateDebugState(DebugType dt);
DebugType getDebugState();

// template <typename... Args> void log(Args... args) {
//     log<Args...>(DebugType::GENERAL, std::cout, args...);
// }
// template <typename... Args> void log(std::ostream& os, Args... args) {
//     log<Args...>(DebugType::GENERAL, os, args...);
// }
// template <typename... Args> void log(DebugType dt, Args... args) {
//     log<Args...>(dt, std::cout, args...);
// }
// template <typename Arg0> void log([[maybe_unused]]DebugType dt,
// [[maybe_unused]]std::ostream& os, [[maybe_unused]]Arg0 arg0) { 
    // #if defined(DEBUG) && DEBUG==1
//     if(checkDebugState(dt)) os << arg0;
// #endif
// }
// template <typename Arg0, typename Arg1, typename... Args>
// void log(DebugType dt, std::ostream& os, Arg0 arg0, Arg1 arg1, Args... args)
// {
//     log<Arg0>(dt, os, arg0);
//     log<Arg1, Args...>(dt, os, arg1, args...);
// }

} // namespace debug
} // namespace common

#endif
