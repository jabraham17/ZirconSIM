#ifndef ZIRCON_COMMON_DEBUG_H_
#define ZIRCON_COMMON_DEBUG_H_

#include <iostream>
#include <string>

#define DEBUG_CATAGORIES(F)                                                    \
    F(NONE, 0x0)                                                               \
    F(LOG, 0x1)                                                                \
    F(PARSER, 0x2)                                                             \
    F(SYSCALL, 0x4)

namespace common {
namespace debug {

struct DebugType {
    using ValueType = int;

  private:
    ValueType value_;

  public:
    // note this is DebugType, not ValueType
    // it must be DebugType, or the templating for log<>(....) will not work
#define TOKEN(d, v) static const DebugType d;
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
    std::string logPrefix() {
        // if it matches exactly NONE, return nothing
        // if it matches exactly ALL, return "ALL"
        // if not, extract out each piece and return them
        if(this->value_ == NONE) return "";
        std::string prefix;
        std::string sep;
        bool matches_all = true;
#define DEBUG_CASE(d, v)                                                       \
    if(this->value_ & d) {                                                     \
        prefix = prefix + sep + #d;                                            \
        sep = "|";                                                             \
    } else {                                                                   \
        matches_all = false;                                                   \
    }
        DEBUG_CATAGORIES(DEBUG_CASE)
#undef DEBUG_CASE

        if(matches_all) return "<ALL>: ";
        else return prefix + ": ";
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

namespace details {
class LogHelper {
  public:
    template <typename... Args>
    static void
    log([[maybe_unused]] DebugType dt,
        [[maybe_unused]] std::ostream& os,
        [[maybe_unused]] Args... args) {
#if defined(DEBUG) && DEBUG == 1
        if(checkDebugState(dt)) {
            os << dt.logPrefix();
            LogHelper::logHelper(dt, os, args...);
        }
#endif
    }
#if defined(DEBUG) && DEBUG == 1
  private:
    template <typename Arg0, typename... Args>
    static void logHelper(
        [[maybe_unused]] DebugType dt,
        std::ostream& os,
        Arg0 arg0,
        Args... args) {
        LogHelper::logHelper(dt, os, arg0);
        LogHelper::logHelper(dt, os, args...);
    }
    template <typename Arg0>
    static void
    logHelper([[maybe_unused]] DebugType dt, std::ostream& os, Arg0 arg0) {
        os << arg0;
    }
#endif
};
} // namespace details

template <typename... Args> void log(DebugType dt, Args... args) {
    details::LogHelper::log(dt, std::cout, args...);
}
template <typename... Args> void log(std::ostream& os, Args... args) {
    details::LogHelper::log(DebugType::LOG, os, args...);
}
template <typename... Args> void log(Args... args) {
    details::LogHelper::log(DebugType::LOG, std::cout, args...);
}
template <typename... Args> void logln(DebugType dt, Args... args) {
    details::LogHelper::log(dt, std::cout, args..., "\n");
}
template <typename... Args> void logln(std::ostream& os, Args... args) {
    details::LogHelper::log(DebugType::LOG, os, args..., "\n");
}
template <typename... Args> void logln(Args... args) {
    details::LogHelper::log(DebugType::LOG, std::cout, args..., "\n");
}

class CancelableOStream {
  private:
    bool isEnabled;
    std::ostream& os;

  public:
    CancelableOStream(bool isEnabled, std::ostream& os)
        : isEnabled(isEnabled), os(os) {}
    ~CancelableOStream() = default;
    CancelableOStream(const CancelableOStream& other) = delete;
    CancelableOStream(CancelableOStream&& other) noexcept = default;
    CancelableOStream& operator=(const CancelableOStream& other) = delete;
    CancelableOStream& operator=(CancelableOStream&& other) noexcept = delete;

    template <typename T> CancelableOStream& operator<<(T&& x) {
        if(isEnabled) os << std::forward<T>(x);
        return *this;
    }

    // special case for ostream inits
    using OStreamType = std::basic_ostream<char, std::char_traits<char>>;
    using OStreamManip = OStreamType& (*)(OStreamType&);
    CancelableOStream& operator<<(OStreamManip manip) {
        if(isEnabled) manip(os);
        return *this;
    }
};

CancelableOStream rawlog(DebugType dt, std::ostream& os);
CancelableOStream rawlog(DebugType dt);
CancelableOStream rawlog(std::ostream& os);
CancelableOStream rawlog();

} // namespace debug
} // namespace common

#endif
