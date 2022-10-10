
#ifndef ZIRCON_TRACE_TRACE_H_
#define ZIRCON_TRACE_TRACE_H_

#include <iomanip>
#include <iostream>
#include <ostream>

struct TraceMode {
    using ValueType = int;
    ValueType _value;
    static const ValueType NONE = 0x0;
    static const ValueType INSTRUCTION = 0x1;
    static const ValueType MEMORY = 0x2;
    static const ValueType REGISTER = 0x4;

    TraceMode(ValueType v) : _value(v) {}
    TraceMode() : _value(NONE) {}

    TraceMode& operator=(const ValueType& v) {
        this->_value = v;
        return *this;
    }
    operator ValueType() const { return this->_value; }

    friend std::ostream& operator<<(std::ostream& o, const TraceMode& tm) {
        if(tm & TraceMode::INSTRUCTION) {
            o << "INSTRUCTION|";
        }
        if(tm & TraceMode::MEMORY) {
            o << "MEMORY|";
        }
        if(tm & TraceMode::REGISTER) {
            o << "REGISTER|";
        }
        return o;
    }

    // operator bool() const { return bool(this->_value); }

    // friend TraceMode operator|(const TraceMode& lhs, const TraceMode& rhs) {
    //     return TraceMode(lhs._value | rhs._value);
    // }
    // friend TraceMode operator|=(TraceMode lhs, const TraceMode& rhs) {
    //     return lhs | rhs;
    // }
    // friend TraceMode operator&(TraceMode lhs, TraceMode rhs) {
    //     return TraceMode(lhs._value & rhs._value);
    // }
    // friend TraceMode operator&=(TraceMode lhs, const TraceMode& rhs) {
    //     return lhs & rhs;
    // }
    // friend TraceMode operator~(const TraceMode& a) {
    //     return TraceMode(a._value);
    // }
};

class Trace {
  private:
    std::string name;
    std::ostream& out;
    bool enabled;

  public:
    Trace(const std::string& name, std::ostream& o = std::cout)
        : name(name), out(o), enabled(false) {}
    void enable() { enabled = true; }
    void disable() { enabled = false; }
    void setState(bool s) { enabled = s; }
    template <class T> Trace& operator<<(const T& x) {
        if(enabled) out << x;
        return *this;
    }

    using TraceManip = Trace& (*)(Trace&);
    Trace& operator<<(TraceManip manip) { return manip(*this); }

    static Trace& flush(Trace& stream) {
        stream.out.flush();
        return stream;
    }
    static Trace& byte(Trace& stream) {

        stream << "0x";
        stream << std::setfill('0');
        stream << std::setw(2);
        stream << std::right;
        stream << std::hex;

        return stream;
    }
    static Trace& halfword(Trace& stream) {

        stream << "0x";
        stream << std::setfill('0');
        stream << std::setw(4);
        stream << std::right;
        stream << std::hex;

        return stream;
    }
    static Trace& word(Trace& stream) {

        stream << "0x";
        stream << std::setfill('0');
        stream << std::setw(8);
        stream << std::right;
        stream << std::hex;

        return stream;
    }
    static Trace& doubleword(Trace& stream) {
        stream << "0x";
        stream << std::setfill('0');
        stream << std::setw(16);
        stream << std::right;
        stream << std::hex;

        return stream;
    }

    using OStreamType = std::basic_ostream<char, std::char_traits<char>>;
    using OStreamManip = OStreamType& (*)(OStreamType&);
    Trace& operator<<(OStreamManip manip) {
        if(enabled) manip(out);
        return *this;
    }
};

#endif
