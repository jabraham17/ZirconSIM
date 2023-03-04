
#ifndef ZIRCON_CPU_REGISTER_H_
#define ZIRCON_CPU_REGISTER_H_

#include "common/format.h"
#include "event/event.h"
#include "hart/types.h"

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

constexpr static types::UnsignedInteger getMask(types::UnsignedInteger N) {
    assert(N <= sizeof(types::UnsignedInteger) * 8);
    return 0xFFFFFFFFFFFFFFFFULL >> ((sizeof(types::UnsignedInteger) * 8) - N);
}

class RegisterClass;
class Register {
    friend RegisterClass;

  private:
    size_t num_bits;
    types::UnsignedInteger value;
    std::string name;
    bool rd_only;

  public:
    Register(size_t num_bits, std::string name = "", bool rd_only = false)
        : Register(num_bits, 0, name, rd_only) {}
    Register(
        size_t num_bits,
        types::UnsignedInteger value,
        std::string name = "",
        bool rd_only = false)
        : num_bits(num_bits), value(value), name(name), rd_only(rd_only) {
        assert(num_bits <= sizeof(types::UnsignedInteger) * 8);
    }

    // cannot copy, only move
    Register() : num_bits(0), value(), name(), rd_only() {}
    Register(const Register&) = delete;
    Register(Register&&) = default;
    Register& operator=(const Register&) = delete;
    Register& operator=(Register&&) = default;
    ~Register() = default;

    void set(const types::UnsignedInteger& value) {
        if(!rd_only) this->value = value & getMask(num_bits);
    }
    Register& operator=(const types::UnsignedInteger& value) {
        this->set(value);
        return *this;
    }

    types::UnsignedInteger get() const { return value & getMask(num_bits); }
    operator types::UnsignedInteger() const { return get(); }

    const std::string& getName() { return name; }
    void dump(std::ostream& o) {
        o << std::setfill(' ') << std::setw(4) << getName() << " "
          << common::Format::doubleword << get();
    }
};

class RegisterClass {
  private:
    std::string classname;
    std::string regprefix;
    size_t num_registers;
    std::unique_ptr<Register[]> registers;

    struct RegisterProxy {
        using T = uint64_t;

      private:
        RegisterClass* rc;
        size_t reg_idx;
        friend RegisterClass;

      public:
        T read() { return rc->registers[reg_idx].get(); }
        void write(T v) { rc->registers[reg_idx].set(v); }
        RegisterProxy(RegisterClass* rc, unsigned reg_idx)
            : rc(rc), reg_idx(reg_idx) {}
        operator T() {
            T v = read();
            rc->event_read(rc->classname, reg_idx, v);
            return v;
        }
        RegisterProxy& operator=(T v) {
            T old_value = read();
            write(v);
            rc->event_write(rc->classname, reg_idx, v, old_value);
            return *this;
        }
    };

    // Subsystem: reg_$CLASS
    // Description: Fires when a register is read
    // Parameters: (register class, register index, value read)
    event::Event<std::string, uint64_t, uint64_t> event_read;
    // Subsystem: reg_$CLASS
    // Description: Fires when a register is written
    // Parameters: (register class, register index, value written, old value)
    event::Event<std::string, uint64_t, uint64_t, uint64_t> event_write;

  public:
    template <typename... RegisterArgs>
    RegisterClass(
        std::string classname,
        std::string regprefix,
        size_t num_registers,
        RegisterArgs&&... init_registers)
        : classname(classname), regprefix(regprefix),
          num_registers(num_registers),
          registers(std::make_unique<Register[]>(num_registers)) {
        // start at index 0 and init all registers
        initRegisters(0, std::forward<RegisterArgs>(init_registers)...);
    }

  private:
    template <typename RegisterArg>
    void initRegisters(size_t idx, RegisterArg reg) {
        assert(idx < num_registers);
        this->registers[idx] = std::move(reg);
    }
    template <typename RegisterArg, typename... RegisterArgs>
    void initRegisters(
        size_t idx,
        RegisterArg reg,
        RegisterArgs&&... init_registers) {
        initRegisters(idx, std::forward<RegisterArg>(reg));
        initRegisters(idx + 1, std::forward<RegisterArgs>(init_registers)...);
    }

  public:
    // cannot copy, only move
    RegisterClass()
        : classname(), regprefix(), num_registers(0), registers(nullptr) {}
    RegisterClass(const RegisterClass&) = delete;
    RegisterClass(RegisterClass&&) = default;
    RegisterClass& operator=(const RegisterClass&) = delete;
    RegisterClass& operator=(RegisterClass&&) = default;
    ~RegisterClass() = default;

    Register& rawreg(unsigned idx) {
        assert(registers && idx < num_registers);
        return registers[idx];
    }

    RegisterProxy reg(unsigned idx) {
        assert(registers && idx < num_registers);
        return RegisterProxy(this, idx);
    }
    RegisterProxy operator[](unsigned idx) { return reg(idx); }

    const std::string& getName() { return classname; }
    void dump(std::ostream& o) {
        o << getName();
        auto quart = num_registers / 4;
        for(size_t i = 0; i < quart; i++) {
            o << "\n";
            rawreg(i).dump(o);
            o << " ";
            rawreg(i + quart).dump(o);
            o << " ";
            rawreg(i + quart + quart).dump(o);
            o << " ";
            rawreg(i + quart + quart + quart).dump(o);
        }
    }

    void addReadListener(
        event::Event<std::string, uint64_t, uint64_t>::callback_type func) {
        event_read.addListener(func);
    }
    void addWriteListener(
        event::Event<std::string, uint64_t, uint64_t, uint64_t>::callback_type
            func) {
        event_write.addListener(func);
    }
};

#endif
