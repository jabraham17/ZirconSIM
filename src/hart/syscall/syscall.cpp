#include "syscall.h"

#include "common/debug.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
    #include <syscall_arch.h>
#endif

namespace sys {

namespace internal {

// takes a simulated address and converts it to a real address
template <typename T>
T convertToRealAddress(hart::HartState& hs, types::Address addr) {
    if(addr) return T(hs().mem().raw(addr));
    else return T(0);
}

int64_t
getMappedSyscallNumber([[maybe_unused]] int64_t riscv64_syscall_number) {
#define MAP_SYSCALL(name, x86_64, riscv64, ...)                                \
    if(riscv64_syscall_number == riscv64) {                                    \
        common::debug::logln(                                                  \
            common::debug::DebugType::SYSCALL,                                 \
            "Emulating " #name " " #riscv64 " as " #x86_64);                   \
        return x86_64;                                                         \
    }
#include "syscall.inc"
    return -1;
}

bool emulateSyscall(
    [[maybe_unused]] uint64_t sys,
    [[maybe_unused]] hart::HartState& hs) {
#define EMULATE_SYSCALL(name, riscv64, execution, ...)                         \
    if(sys == riscv64) {                                                       \
        common::debug::logln(                                                  \
            common::debug::DebugType::SYSCALL,                                 \
            "Emulating " #name "[" #riscv64 "]",                               \
            " arg0=",                                                          \
            common::Format::doubleword,                                        \
            hs().rf().GPR.rawreg(10).get(),                                    \
            " arg1=",                                                          \
            common::Format::doubleword,                                        \
            hs().rf().GPR.rawreg(11).get(),                                    \
            " arg2=",                                                          \
            common::Format::doubleword,                                        \
            hs().rf().GPR.rawreg(12).get(),                                    \
            " arg3=",                                                          \
            common::Format::doubleword,                                        \
            hs().rf().GPR.rawreg(13).get(),                                    \
            " arg4=",                                                          \
            common::Format::doubleword,                                        \
            hs().rf().GPR.rawreg(14).get(),                                    \
            " arg5=",                                                          \
            common::Format::doubleword,                                        \
            hs().rf().GPR.rawreg(15).get());                                   \
        do {                                                                   \
            execution;                                                         \
        } while(0);                                                            \
        common::debug::logln(                                                  \
            common::debug::DebugType::SYSCALL,                                 \
            "Syscall " #name " returned with ",                                \
            common::Format::doubleword,                                        \
            hs().rf().GPR.rawreg(10).get());                                   \
        return true;                                                           \
    }
#include "syscall.inc"
    return false;
}

} // namespace internal

void emulate(hart::HartState& hs) {
    uint64_t riscv64_syscall_number = hs().rf().GPR[17];
    uint64_t result;

    int64_t x86_64_syscall_number =
        internal::getMappedSyscallNumber(riscv64_syscall_number);

    if(x86_64_syscall_number == -1) {
        if(internal::emulateSyscall(riscv64_syscall_number, hs)) return;
        else throw SyscallUnimplementedException(riscv64_syscall_number);
    }

#ifdef __x86_64
    uint64_t arg0 = hs().rf().GPR[10];
    uint64_t arg1 = hs().rf().GPR[11];
    uint64_t arg2 = hs().rf().GPR[12];
    uint64_t arg3 = hs().rf().GPR[13];
    uint64_t arg4 = hs().rf().GPR[14];
    uint64_t arg5 = hs().rf().GPR[15];
    asm volatile(
    #if(defined(__clang__) && defined(__clang_major__) &&                      \
        __clang_major__ >= 14) ||                                              \
        (!defined(__clang__) && (defined(__GNUC__) || defined(__GNUG__)))
        "mov rax, %[sys]\n\t"
        "mov rdi, %[arg0]\n\t"
        "mov rsi, %[arg1]\n\t"
        "mov rdx, %[arg2]\n\t"
        "mov r10, %[arg3]\n\t"
        "mov r8, %[arg4]\n\t"
        "mov r9, %[arg5]\n\t"
        "syscall\n\t"
        "mov %[result], rax\n\t"
    #else
        "movq %[sys], %%rax\n\t"
        "movq %[arg0], %%rdi\n\t"
        "movq %[arg1], %%rsi\n\t"
        "movq %[arg2], %%rdx\n\t"
        "movq %[arg3], %%r10\n\t"
        "movq %[arg4], %%r8\n\t"
        "movq %[arg5], %%r9\n\t"
        "syscall\n\t"
        "movq %%rax, %[result]\n\t"
    #endif
        : [result] "=r"(result)
        : [arg0] "r"(arg0),
          [arg1] "r"(arg1),
          [arg2] "r"(arg2),
          [arg3] "r"(arg3),
          [arg4] "r"(arg4),
          [arg5] "r"(arg5),
          [sys] "r"(x86_64_syscall_number)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9");
#else
    //     #ifdef __EMSCRIPTEN__
    //     uint64_t arg0 = hs().rf().GPR[10];
    //     uint64_t arg1 = hs().rf().GPR[11];
    //     uint64_t arg2 = hs().rf().GPR[12];
    //     uint64_t arg3 = hs().rf().GPR[13];
    //     uint64_t arg4 = hs().rf().GPR[14];
    //     uint64_t arg5 = hs().rf().GPR[15];

    //     result = __syscall_emscripten(x86_64_syscall_number, arg0, arg1,
    //     arg2, arg3, arg4, arg5); #else
    throw SyscallUnimplementedException(riscv64_syscall_number);
// #endif
#endif
    hs().rf().GPR[10] = result;
}

} // namespace sys
