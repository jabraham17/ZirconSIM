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

// rv64ima-linux-toolchain/sysroot/usr/include/asm-generic/unistd.h

#define DO_NOTHING                                                             \
    do {                                                                       \
    } while(0)

// MAP_SYSCALL(read, 0, 63, fd, buf, count)
// MAP_SYSCALL(write, 1, 64, fd, buf, count)
// MAP_SYSCALL(close, 3, 57, fd)
MAP_SYSCALL(getegid, 108, 177)
MAP_SYSCALL(getgid, 104, 176)
MAP_SYSCALL(geteuid, 107, 175)
MAP_SYSCALL(getuid, 102, 174)

EMULATE_SYSCALL(fstat, 80, uint64_t fildes = hs.rf.GPR[10];
                uint64_t addr = hs.rf.GPR[11];
                struct stat* real_addr = (struct stat*)hs.memimg.raw(addr);
                hs.rf.GPR[10] = fstat(fildes, real_addr);)

EMULATE_SYSCALL(write, 64, uint64_t fd = hs.rf.GPR[10];
                uint64_t addr = hs.rf.GPR[11];
                void* real_addr = (void*)hs.memimg.raw(addr);
                uint64_t count = hs.rf.GPR[12];
                hs.rf.GPR[10] = write(fd, real_addr, count);)

EMULATE_SYSCALL(
    gettimeofday, 169, uint64_t addr = hs.rf.GPR[10];
    struct timeval* tv = addr ? (struct timeval*)hs.memimg.raw(addr) : 0;
    addr = hs.rf.GPR[11];
    struct timezone* tz = addr ? (struct timezone*)hs.memimg.raw(addr) : 0;
    hs.rf.GPR[10] = gettimeofday(tv, tz);)

EMULATE_SYSCALL(exit, 93, DO_NOTHING;)

// brk called with 0 returns the end of the heap
EMULATE_SYSCALL(
    brk, 214, uint64_t addr = uint64_t(hs.rf.GPR[10]); if(addr != 0) {
        uint64_t allocation_size = addr - hs.memory_locations["heap_end"];
        hs.memimg.allocate(hs.memory_locations["heap_end"], allocation_size);
        hs.memory_locations["heap_end"] = addr;
    } hs.rf.GPR[10] = hs.memory_locations["heap_end"];

)
// EMULATE_SYSCALL(close, 57, hs.rf.GPR[10] = -1;)

#undef MAP_SYSCALL
#undef EMULATE_SYSCALL
