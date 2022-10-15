#ifndef MAP_SYSCALL
    #define MAP_SYSCALL(name, x86_64, riscv64, ...)
#endif
#ifndef EMULATE_SYSCALL
    #define EMULATE_SYSCALL(name, riscv64, execution, ...)
#endif

// x86 args
//  rdi, rsi, rdx, r10, r8, r9
// syscall #: rax

// riscv64 args
// a0, a1, a2, a3, a4, a5
// SYSCALL #: a7

#define DO_NOTHING                                                             \
    do {                                                                       \
    } while(0)

// MAP_SYSCALL(read, 0, 63, fd, buf, count)
// MAP_SYSCALL(write, 1, 64, fd, buf, count)
// MAP_SYSCALL(close, 3, 57, fd)
// MAP_SYSCALL(fstat, 5, 80)

EMULATE_SYSCALL(fstat, 80, uint64_t fildes = hs.rf.GPR[10];
                uint64_t addr = hs.rf.GPR[11];
                struct stat* real_addr = (struct stat*)hs.memimg.raw(addr);
                hs.rf.GPR[10] = fstat(fildes, real_addr);)

EMULATE_SYSCALL(write, 64, uint64_t fd = hs.rf.GPR[10];
                uint64_t addr = hs.rf.GPR[11];
                void* real_addr = (void*)hs.memimg.raw(addr);
                uint64_t count = hs.rf.GPR[12];
                hs.rf.GPR[10] = write(fd, real_addr, count);)

EMULATE_SYSCALL(exit, 93, DO_NOTHING;)
EMULATE_SYSCALL(brk, 214, hs.rf.GPR[10] = -1;)
EMULATE_SYSCALL(close, 57, hs.rf.GPR[10] = -1;)

#undef MAP_SYSCALL
#undef EMULATE_SYSCALL
