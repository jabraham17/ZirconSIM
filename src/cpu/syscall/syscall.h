#ifndef SRC_CPU_SYSCALL_SYSCALL_H_
#define SRC_CPU_SYSCALL_SYSCALL_H_

#include "cpu/cpu.h"
#include <cstdint>
#include <cstring>
#include <exception>
#include <string>

namespace sys {

namespace internal {

extern int64_t getEmulatedSyscallNumber(int64_t riscv64_syscall_number);
}

struct SyscallUnimplementedException : public std::exception {
    int riscv64_syscall_number;
    SyscallUnimplementedException(int riscv64_syscall_number)
        : riscv64_syscall_number(riscv64_syscall_number) {}
    const char* what() const noexcept {
        std::string s(
            "Syscall " + std::to_string(riscv64_syscall_number) +
            " Not Implemented");
        return strdup(s.c_str());
    }
};

void emulate(cpu::HartState& hs);

} // namespace sys

#endif
