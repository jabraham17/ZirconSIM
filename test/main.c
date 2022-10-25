
/*
./toolchains/rv64ima/bin/riscv64-unknown-elf-gcc -g -static test/main.c -o test/main-elf.out && ./toolchains/rv64ima-musl/bin/riscv64-unknown-linux-musl-gcc -g -static test/main.c -o test/main-musl.out && ./toolchains/rv64ima-linux/bin/riscv64-unknown-linux-gnu-gcc -g -static test/main.c -o test/main-linux.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

int ack(int m, int n)
{
    if (m == 0){
        return n+1;
    }
    else if((m > 0) && (n == 0)){
        return ack(m-1, 1);
    }
    else if((m > 0) && (n > 0)){
        return ack(m-1, ack(m, n-1));
    }
}

int fib(int n) {
    if(n <= 0) return 0;
    if(n == 1) return 1;
    if(n==2) return 1;
    return fib(n-1) + fib(n-2);
}

int main() {
    srand(time(0));
    printf("hello, world %d\n", rand());

    // printf("fib(19)=%d\n",fib(19));

    // printf("ack(3,4)=%d\n",ack(3,4));
    fflush(stdout);

    int* x = (int*)malloc(sizeof(int) * 10);
    x[0] = 10;
    if(x == NULL) {
        int errsv = errno;
        perror("x is NULL");
        printf("err %d\n", errsv);
        return 0;
    }
    printf("%p\n", x);
    for(int i = 0; i < 10; i++) {
        // x[i] = i * i;
    }

    // asm("1: j 1b\n");
    return 0;
}
