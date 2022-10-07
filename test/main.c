
/*
clang --target=riscv64 -march=rv64i test/main.c -c -o test/main.o &&
/opt/riscv-ima/bin/riscv64-unknown-elf-gcc -static test/main.o -o test/main.out
*/
int global = 7;
int main() { return global; }
