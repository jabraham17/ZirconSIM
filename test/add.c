// compile:
/*clang --target=riscv64 -march=rv64i -O3 -c test/add.c -o test/add.o &&
 * llvm-objcopy -O binary --only-section=.text test/add.o test/add.bin */
long add(long a, long b) { return a + b; }
