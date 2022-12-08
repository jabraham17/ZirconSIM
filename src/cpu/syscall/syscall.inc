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

#ifndef __EMSCRIPTEN__
MAP_SYSCALL(ioctl, 54, 29)
#else
EMULATE_SYSCALL(ioctl,
                29,
                hs().rf().GPR[10] = __syscall_ioctl(
                    hs().rf().GPR[10],
                    hs().rf().GPR[11],
                    hs().rf().GPR[12],
                    hs().rf().GPR[13],
                    hs().rf().GPR[14],
                    hs().rf().GPR[15]);)
#endif

EMULATE_SYSCALL(fstat,
                80,
                // uint64_t fildes = hs().rf().GPR[10];
                // struct stat* addr = convertToRealAddress<struct stat*>(hs,
                // hs().rf().GPR[11]);
                hs().rf().GPR[10] = -1;)

EMULATE_SYSCALL(
    open, 1024,
    const char* path = convertToRealAddress<const char*>(hs, hs().rf().GPR[10]);
    int flags = int(hs().rf().GPR[11]);
    mode_t mode;
    // if create or tmp, use mode
    // else use a mode of 0
    if(flags & O_CREAT || flags & O_TMPFILE)
    mode = mode_t(hs().rf().GPR[12]);
    else mode = 0;
    // printf("ZIRCON DEBUG: %s 0x%x 0x%x\n", path, flags, mode);
    // printf("O_APPEND %d\n", flags & O_APPEND); 
    // printf("O_ASYNC %d\n", flags & O_ASYNC); 
    // printf("O_CLOEXEC %d\n", flags & O_CLOEXEC); 
    // printf("O_CREAT %d\n", flags & O_CREAT); 
    // printf("O_DIRECT %d\n", flags & O_DIRECT); 
    // printf("O_DIRECTORY %d\n", flags & O_DIRECTORY); 
    // printf("O_DSYNC %d\n", flags & O_DSYNC); 
    // printf("O_EXCL %d\n", flags & O_EXCL); 
    // printf("O_LARGEFILE %d\n", flags & O_LARGEFILE); 
    // printf("O_NOATIME %d\n", flags & O_NOATIME); 
    // printf("O_NOCTTY %d\n", flags & O_NOCTTY); 
    // printf("O_NOFOLLOW %d\n", flags & O_NOFOLLOW); 
    // printf("O_NONBLOCK %d\n", flags & O_NONBLOCK); 
    // printf("O_NDELAY %d\n", flags & O_NDELAY); 
    // printf("O_PATH %d\n", flags & O_PATH); 
    // printf("O_SYNC %d\n", flags & O_SYNC); 
    // printf("O_TMPFILE %d\n", flags & O_TMPFILE); 
    // printf("O_TRUNC %d\n", flags & O_TRUNC); 
    hs().rf().GPR[10] = syscall(SYS_open, path, flags, mode);
    )

EMULATE_SYSCALL(
    openat, 56, int fd = hs().rf().GPR[10];
    const char* path = convertToRealAddress<const char*>(hs, hs().rf().GPR[11]);
    int oflag = hs().rf().GPR[12];
    hs().rf().GPR[10] =
        openat(fd, path, oflag, hs().rf().GPR[13], hs().rf().GPR[14], hs().rf().GPR[15]);)

EMULATE_SYSCALL(close, 57, uint64_t fd = hs().rf().GPR[10];
                // dont close stdio
                // FIXME: ugly hack, maybe we should emulate these files?
                if(fd != 0 && fd != 1 && fd != 2) hs().rf().GPR[10] = close(fd);)

EMULATE_SYSCALL(lseek, 62, uint64_t fd = hs().rf().GPR[10];
                off_t offset = off_t(hs().rf().GPR[11]);
                int whence = hs().rf().GPR[12];
                hs().rf().GPR[10] = lseek(fd, offset, whence);)

EMULATE_SYSCALL(read, 63, uint64_t fd = hs().rf().GPR[10];
                void* addr = convertToRealAddress<void*>(hs, hs().rf().GPR[11]);
                uint64_t count = hs().rf().GPR[12];
                hs().rf().GPR[10] = read(fd, addr, count);)

EMULATE_SYSCALL(write, 64, uint64_t fd = hs().rf().GPR[10];
                void* addr = convertToRealAddress<void*>(hs, hs().rf().GPR[11]);
                uint64_t count = hs().rf().GPR[12];
                hs().rf().GPR[10] = write(fd, addr, count);)

// EMULATE_SYSCALL(
//     writev, 66, uint64_t fildes = hs().rf().GPR[10];
//     struct iovec* iov = convertToRealAddress<struct iovec*>(hs,
//     hs().rf().GPR[11], true); uint64_t iovcnt = hs().rf().GPR[12];
//     // need to rewrite the address inside of iov
//     for(uint64_t idx = 0; idx < iovcnt; idx++) {
//         if(iov &&
//            hs().mem().raw((uint64_t)iov + (idx * 16) + 8
//            /*iov[idx].iov_len*/)) {
//             uint64_t orig_buffer = *(uint64_t*)(hs().mem().raw(
//                 (uint64_t)iov + (idx * 16) /*iov[idx].iov_base*/));
//             *(uint64_t*)(hs().mem().raw(
//                 (uint64_t)iov + (idx * 16) /*iov[idx].iov_base*/)) =
//                 convertToRealAddress<uint64_t>(hs, orig_buffer, true);
//         }
//     } hs().rf().GPR[10] = writev(fildes, iov, iovcnt);)
EMULATE_SYSCALL(
    writev, 66, uint64_t fildes = hs().rf().GPR[10];
    struct iovec* iov = convertToRealAddress<struct iovec*>(hs, hs().rf().GPR[11]);
    uint64_t iovcnt = hs().rf().GPR[12];
    // need to rewrite the address inside of iov
    for(uint64_t idx = 0; idx < iovcnt; idx++) {
        if(iov && iov[idx].iov_len) {
            uint64_t orig_buffer = uint64_t(iov[idx].iov_base);
            iov[idx].iov_base = convertToRealAddress<void*>(hs, orig_buffer);
        }
    } hs().rf().GPR[10] = writev(fildes, iov, iovcnt);)

EMULATE_SYSCALL(clock_settime,
                112,
                clockid_t clockid = (clockid_t)hs().rf().GPR[10];
                struct timespec* tp =
                    convertToRealAddress<struct timespec*>(hs, hs().rf().GPR[11]);
                hs().rf().GPR[10] = clock_settime(clockid, tp);)
EMULATE_SYSCALL(clock_gettime,
                113,
                clockid_t clockid = (clockid_t)hs().rf().GPR[10];
                struct timespec* tp =
                    convertToRealAddress<struct timespec*>(hs, hs().rf().GPR[11]);
                hs().rf().GPR[10] = clock_gettime(clockid, tp);)
EMULATE_SYSCALL(clock_getres, 114, clockid_t clockid = (clockid_t)hs().rf().GPR[10];
                struct timespec* res =
                    convertToRealAddress<struct timespec*>(hs, hs().rf().GPR[11]);
                hs().rf().GPR[10] = clock_getres(clockid, res);)

EMULATE_SYSCALL(uname,
                160,
                struct utsname* buf =
                    convertToRealAddress<struct utsname*>(hs, hs().rf().GPR[10]);
                hs().rf().GPR[10] = uname(buf);)

EMULATE_SYSCALL(getrlimit, 163, uint64_t resource = hs().rf().GPR[10];
                struct rlimit* rlp =
                    convertToRealAddress<struct rlimit*>(hs, hs().rf().GPR[11]);
                hs().rf().GPR[10] = getrlimit(resource, rlp);)

EMULATE_SYSCALL(setrlimit, 164, uint64_t resource = hs().rf().GPR[10];
                struct rlimit* rlp =
                    convertToRealAddress<struct rlimit*>(hs, hs().rf().GPR[11]);
                hs().rf().GPR[10] = setrlimit(resource, rlp);)

EMULATE_SYSCALL(umask, 166, uint64_t mode = hs().rf().GPR[10];
                hs().rf().GPR[10] = umask(mode);)

EMULATE_SYSCALL(gettimeofday,
                169,
                struct timeval* tv =
                    convertToRealAddress<struct timeval*>(hs, hs().rf().GPR[10]);
                struct timezone* tz =
                    convertToRealAddress<struct timezone*>(hs, hs().rf().GPR[11]);
                hs().rf().GPR[10] = gettimeofday(tv, tz);)

EMULATE_SYSCALL(exit, 93, hs().executing = false;)
EMULATE_SYSCALL(exit_group, 94, hs().executing = false;)

#ifndef __EMSCRIPTEN__
EMULATE_SYSCALL(set_tid_address,
                96,
                int* tidptr = convertToRealAddress<int*>(hs, hs().rf().GPR[10]);
                hs().rf().GPR[10] = syscall(SYS_set_tid_address, tidptr);)
#else
// FIXME: no concept of set_tid_address in emscripten, whats the better
// translation here
EMULATE_SYSCALL(set_tid_address, 96, hs().rf().GPR[10] = gettid())
#endif
// brk called with 0 returns the end of the heap
EMULATE_SYSCALL(
    brk, 214, uint64_t addr = uint64_t(hs().rf().GPR[10]); if(addr != 0) {
        uint64_t allocation_size = addr - hs().getMemLocation("heap_end");
        hs().mem().allocate(hs().getMemLocation("heap_end"), allocation_size);
        hs().setMemLocation("heap_end", addr);
    } hs().rf().GPR[10] = hs().getMemLocation("heap_end");

)
// EMULATE_SYSCALL(close, 57, hs().rf().GPR[10] = -1;)

EMULATE_SYSCALL(
    mmap, 222, uint64_t addr = uint64_t(hs().rf().GPR[10]); if(addr == 0) {
        size_t length = size_t(hs().rf().GPR[11]);
        if(length == 0) {
            hs().rf().GPR[10] = -EINVAL;
            ;
        } else {
            // int prot = int(hs().rf().GPR[12]);
            int flags = int(hs().rf().GPR[13]);
            int fd = uint64_t(hs().rf().GPR[14]);
            off_t offset = off_t(hs().rf().GPR[15]);

            addr = hs().getMemLocation("heap_end");
            hs().mem().allocate(addr, length);
            hs().setMemLocation("heap_end", addr + length);

            // hs().rf().GPR[10] = uint64_t(mmap(addr, length, prot, flags, fd,
            // offset));
            if(!(flags & MAP_ANONYMOUS)) {
                // init with fd
                lseek(fd, offset, SEEK_SET);
                (void)!read(fd, (void*)addr, length);
            }
            hs().rf().GPR[10] = addr;
        }
    } else { throw SyscallUnimplementedException(222); })

#undef MAP_SYSCALL
#undef EMULATE_SYSCALL
