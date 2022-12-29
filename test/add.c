// compile:
/*clang --target=riscv64 -march=rv64i -O3 -c test/add.c -o test/add.o &&
 * llvm-objcopy -O binary --only-section=.text test/add.o test/add.bin */
long add(long a, long b) { return a + b; }


long data;
long data2;
long add2(long a, long b) { return data + data2 + a + b; }

int main() {return 0;}
