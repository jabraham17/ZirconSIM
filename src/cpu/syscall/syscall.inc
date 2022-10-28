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

MAP_SYSCALL(ioctl, 54, 29)

EMULATE_SYSCALL(fstat, 80, uint64_t fildes = hs.rf.GPR[10];
                uint64_t addr = hs.rf.GPR[11];
                struct stat* real_addr = addr
                                             ? (struct stat*)hs.memimg.raw(addr)
                                             : 0;
                hs.rf.GPR[10] = fstat(fildes, real_addr);)

EMULATE_SYSCALL(write, 64, uint64_t fd = hs.rf.GPR[10];
                uint64_t addr = hs.rf.GPR[11];
                void* real_addr = addr ? (void*)hs.memimg.raw(addr) : 0;
                uint64_t count = hs.rf.GPR[12];
                hs.rf.GPR[10] = write(fd, real_addr, count);)

EMULATE_SYSCALL(
    writev, 66, uint64_t fildes = hs.rf.GPR[10]; uint64_t addr = hs.rf.GPR[11];
    struct iovec* iov = addr ? (struct iovec*)hs.memimg.raw(addr) : 0;
    uint64_t iovcnt = hs.rf.GPR[12];
    // need to rewrite the address inside of iov
    for(uint64_t idx = 0; idx < iovcnt; idx++) {
        if(iov) {
            uint64_t orig_buffer = (uint64_t)iov[idx].iov_base;
            iov[idx].iov_base = orig_buffer ? hs.memimg.raw(orig_buffer) : 0;
        }
    } hs.rf.GPR[10] = writev(fildes, iov, iovcnt);)

EMULATE_SYSCALL(
    clock_settime, 112, clockid_t clockid = (clockid_t)hs.rf.GPR[10];
    uint64_t addr = hs.rf.GPR[11];
    struct timespec* tp = addr ? (struct timespec*)hs.memimg.raw(addr) : 0;
    hs.rf.GPR[10] = clock_settime(clockid, tp);)
EMULATE_SYSCALL(
    clock_gettime, 113, clockid_t clockid = (clockid_t)hs.rf.GPR[10];
    uint64_t addr = hs.rf.GPR[11];
    struct timespec* tp = addr ? (struct timespec*)hs.memimg.raw(addr) : 0;
    hs.rf.GPR[10] = clock_gettime(clockid, tp);)
EMULATE_SYSCALL(
    clock_getres, 114, clockid_t clockid = (clockid_t)hs.rf.GPR[10];
    uint64_t addr = hs.rf.GPR[11];
    struct timespec* res = addr ? (struct timespec*)hs.memimg.raw(addr) : 0;
    hs.rf.GPR[10] = clock_getres(clockid, res);)

EMULATE_SYSCALL(
    gettimeofday, 169, uint64_t addr = hs.rf.GPR[10];
    struct timeval* tv = addr ? (struct timeval*)hs.memimg.raw(addr) : 0;
    addr = hs.rf.GPR[11];
    struct timezone* tz = addr ? (struct timezone*)hs.memimg.raw(addr) : 0;
    hs.rf.GPR[10] = gettimeofday(tv, tz);)

EMULATE_SYSCALL(exit, 93, hs.executing = false;)
EMULATE_SYSCALL(exit, 94, hs.executing = false;)

EMULATE_SYSCALL(set_tid_address, 96, uint64_t addr = hs.rf.GPR[10];
                int* tidptr = addr ? (int*)hs.memimg.raw(addr) : 0;
                hs.rf.GPR[10] = syscall(SYS_set_tid_address, tidptr);)

// brk called with 0 returns the end of the heap
EMULATE_SYSCALL(
    brk, 214,
    uint64_t addr = uint64_t(hs.rf.GPR[10]);
    if(addr != 0) {
        uint64_t allocation_size = addr - hs.memory_locations["heap_end"];
        hs.memimg.allocate(hs.memory_locations["heap_end"], allocation_size);
        hs.memory_locations["heap_end"] = addr;
    } hs.rf.GPR[10] = hs.memory_locations["heap_end"];

)
// EMULATE_SYSCALL(close, 57, hs.rf.GPR[10] = -1;)

EMULATE_SYSCALL(
    mmap, 222, 
    uint64_t addr = uint64_t(hs.rf.GPR[10]);
    if(addr == 0) {
        size_t length = size_t(hs.rf.GPR[11]);
        if(length == 0) {
            hs.rf.GPR[10] = -EINVAL;;
        } else {
            int prot = int(hs.rf.GPR[12]);
            int flags = int(hs.rf.GPR[13]);
            int fd = uint64_t(hs.rf.GPR[14]);
            off_t offset = off_t(hs.rf.GPR[15]);

            addr = hs.memory_locations["heap_end"];
            hs.memimg.allocate(addr, length);
            hs.memory_locations["heap_end"] += length;

            // hs.rf.GPR[10] = uint64_t(mmap(addr, length, prot, flags, fd,
            // offset));
            if(!(flags & MAP_ANONYMOUS)) {
                // init with fd
                lseek(fd, offset, SEEK_SET);
                read(fd, (void*)addr, length);
            }
            hs.rf.GPR[10] = addr;
        }
    } else { throw SyscallUnimplementedException(222); }
    )

#undef MAP_SYSCALL
#undef EMULATE_SYSCALL
