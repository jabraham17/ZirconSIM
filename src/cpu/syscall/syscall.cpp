#include "syscall.h"

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

// template <typename T>
// T convertAddress(cpu::HartState& hs, uint64_t addr, bool checkStack = true) {
//     // if address is within the range of the stack, it needs no change
//     // if the address is no in the range of the stack and it is NOT null,
//     // convert through the memory interface
//     // FIXME: this is likely very very fragile and will possibly break in the
//     // future
//     if(checkStack && addr >= hs.memory_locations["stack_start"] &&
//        addr <= hs.memory_locations["stack_end"]) {
//         return T(addr);
//     } else if(addr == 0) return T(0);
//     else return T(hs.memimg.raw(addr));
// }

// takes a simulated address and converts it to a real address
template <typename T>
T convertToRealAddress(cpu::HartState& hs, uint64_t addr) {
    if(addr) return T(hs.memimg.raw(addr));
    else return T(0);
}

int64_t
getMappedSyscallNumber([[maybe_unused]] int64_t riscv64_syscall_number) {
#define MAP_SYSCALL(name, x86_64, riscv64, ...)                                \
    if(riscv64_syscall_number == riscv64) return x86_64;
#include "syscall.inc"
    return -1;
}

bool emulateSyscall(
    [[maybe_unused]] uint64_t sys,
    [[maybe_unused]] cpu::HartState& hs) {
#define EMULATE_SYSCALL(name, riscv64, execution, ...)                         \
    if(sys == riscv64) {                                                       \
        do {                                                                   \
            execution;                                                         \
        } while(0);                                                            \
        return true;                                                           \
    }
#include "syscall.inc"
    return false;
}

} // namespace internal

void emulate(cpu::HartState& hs) {
    uint64_t riscv64_syscall_number = hs.rf.GPR[17];
    uint64_t result;

    int64_t x86_64_syscall_number =
        internal::getMappedSyscallNumber(riscv64_syscall_number);

    if(x86_64_syscall_number == -1) {
        if(internal::emulateSyscall(riscv64_syscall_number, hs)) return;
        else throw SyscallUnimplementedException(riscv64_syscall_number);
    }

#ifdef __x86_64
    uint64_t arg0 = hs.rf.GPR[10];
    uint64_t arg1 = hs.rf.GPR[11];
    uint64_t arg2 = hs.rf.GPR[12];
    uint64_t arg3 = hs.rf.GPR[13];
    uint64_t arg4 = hs.rf.GPR[14];
    uint64_t arg5 = hs.rf.GPR[15];
    asm volatile(
        #if (defined(__clang__) && defined(__clang_major__) && __clang_major__ >= 14) || (!defined(__clang__) && (defined(__GNUC__) || defined(__GNUG__)))
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
    //     uint64_t arg0 = hs.rf.GPR[10];
    //     uint64_t arg1 = hs.rf.GPR[11];
    //     uint64_t arg2 = hs.rf.GPR[12];
    //     uint64_t arg3 = hs.rf.GPR[13];
    //     uint64_t arg4 = hs.rf.GPR[14];
    //     uint64_t arg5 = hs.rf.GPR[15];

    //     result = __syscall_emscripten(x86_64_syscall_number, arg0, arg1,
    //     arg2, arg3, arg4, arg5); #else
    throw SyscallUnimplementedException(riscv64_syscall_number);
// #endif
#endif
    hs.rf.GPR[10] = result;
}

} // namespace sys
