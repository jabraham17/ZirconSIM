
/*
./toolchains/rv64ima/bin/riscv64-unknown-elf-gcc -g -static test/main.c -o test/main.out
./toolchains/rv64ima-musl/bin/riscv64-unknown-linux-musl-gcc -g -static test/main.c -o test/main.out
./toolchains/rv64ima-linux/bin/riscv64-unknown-linux-gnu-gcc -g -static test/main.c -o test/main.out

clang --target=riscv64 -march=rv64i -c test/main.c -o test/main.o && ./toolchains/rv64ima/bin/riscv64-unknown-elf-gcc -static test/main.o -o test/main.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main() {
    srand(time(0));
    printf("hello, world %d\n", rand());
    // asm("1: j 1b\n");
    return 0;
}
