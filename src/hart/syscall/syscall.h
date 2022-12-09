#ifndef ZIRCON_HART_SYSCALL_SYSCALL_H_
#define ZIRCON_HART_SYSCALL_SYSCALL_H_

#include "hart/hart.h"

#include <cstdint>
#include <cstring>
#include <exception>
#include <string>

namespace sys {

namespace internal {

extern int64_t getEmulatedSyscallNumber(int64_t riscv64_syscall_number);
}

struct SyscallUnimplementedException : public std::runtime_error {
    SyscallUnimplementedException(int riscv64_syscall_number)
        : std::runtime_error(
              "Syscall " + std::to_string(riscv64_syscall_number) +
              " Not Implemented") {}
};

void emulate(hart::HartState& hs);

} // namespace sys

#endif
