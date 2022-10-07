
#ifndef ZIRCON_CPU_REGISTER_H_
#define ZIRCON_CPU_REGISTER_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <string>

template <size_t N> class RegisterFile {

  public:
    RegisterFile(std::string name = "") : name(name) { registers.fill(0); }

    uint64_t& reg(unsigned idx) {
        assert(idx < N);
        if(idx == 0) registers[idx] = 0;
        return registers[idx];
    }
    uint64_t& operator[](unsigned idx) { return reg(idx); }

    const std::string& getName() { return name; }

    void dump(std::ostream& o) {
        o << name << "\n";
        auto half = N / 2;
        for(size_t i = 0; i < half; i++) {
            o << std::dec << std::setfill(' ') << std::setw(4)
              << std::string("x" + std::to_string(i)) << "  0x"
              << std::setfill('0') << std::setw(16) << std::right << std::hex
              << reg(i);

            o << std::setfill(' ') << std::setw(8) << " ";

            o << std::dec << std::setfill(' ') << std::setw(4)
              << std::string("x" + std::to_string(half + i)) << "  0x"
              << std::setfill('0') << std::setw(16) << std::right << std::hex
              << reg(half + i);

            o << "\n";
        }
    }

  private:
    std::string name;
    std::array<uint64_t, N> registers;
};

#endif
