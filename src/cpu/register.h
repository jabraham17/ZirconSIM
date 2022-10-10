
#ifndef ZIRCON_CPU_REGISTER_H_
#define ZIRCON_CPU_REGISTER_H_

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

template <size_t SIZE> class Register {
  private:
    std::bitset<SIZE> value;
    std::string name;
    bool rd_only;

  public:
    Register(std::string name = "", bool rd_only = false)
        : value(0), name(name), rd_only(rd_only) {}

    Register(
        std::bitset<SIZE> bits,
        std::string name = "",
        bool rd_only = false)
        : value(bits), name(name), rd_only(rd_only) {}

    Register(uint64_t value, std::string name = "", bool rd_only = false)
        : value(value), name(name), rd_only(rd_only) {}

    void set(const uint64_t& value) {
        if(!rd_only) this->value = value;
    }
    void set(const std::bitset<SIZE>& value) {
        if(!rd_only) this->value = value;
    }
    Register<SIZE>& operator=(const uint64_t& value) {
        this->set(value);
        return *this;
    }

    std::bitset<SIZE> getBits() { return value; }
    uint64_t get() const {
        assert(SIZE <= 64);
        return value.to_ullong();
    }
    operator uint64_t() const { return get(); }

    const std::string& getName() { return name; }
    void dump(std::ostream& o) {
        o << getName() << " 0x" << std::setfill('0') << std::setw(16)
          << std::right << std::hex << value;
    }
};

template <size_t NUM, size_t SIZE> class RegisterClass {
  private:
    std::string classname;
    std::string regprefix;
    std::array<Register<SIZE>, NUM> registers;

  public:
    RegisterClass(
        std::string classname,
        std::string regprefix,
        std::array<Register<SIZE>, NUM> registers = {})
        : classname(classname), regprefix(regprefix), registers(registers) {}

    RegisterClass() = default;

    Register<SIZE>& reg(unsigned idx) {
        assert(idx < NUM);
        return registers[idx];
    }
    // uint64_t& value(unsigned idx) { return reg(idx)(); }
    Register<SIZE>& operator[](unsigned idx) { return reg(idx); }

    const std::string& getName() { return classname; }
    void dump(std::ostream& o) {
        o << getName();
        auto half = NUM / 2;
        for(size_t i = 0; i < half; i++) {
            o << "\n";
            o << std::setfill(' ') << std::setw(32);
            reg(i).dump(o);
            o << std::setfill(' ') << std::setw(32);
            reg(half + i).dump(o);
        }
    }
};

#endif
