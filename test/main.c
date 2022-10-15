
/*
clang --target=riscv64 -march=rv64i -I/opt/riscv-ima/riscv64-unknown-elf/include
test/main.c -c -o test/main.o && /opt/riscv-ima/bin/riscv64-unknown-elf-gcc
-static test/main.o -o test/main.out
*/
#include <stdio.h>
int global = -2;
int main() {
    printf("hello, world\n");
    return 0;
}
