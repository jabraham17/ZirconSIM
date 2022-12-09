
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

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
    return 0;
}

int fib(int n) {
    if(n <= 0) return 0;
    if(n == 1) return 1;
    if(n==2) return 1;
    return fib(n-1) + fib(n-2);
}

int main() {
    printf("sizeof stat %ld\n", sizeof(struct stat));
    srand(time(0));
    printf("hello, world %d\n", rand());

    printf("fib(19)=%d\n",fib(19));

    printf("ack(3,4)=%d\n",ack(3,4));
    fflush(stdout);

    int* x = (int*)malloc(4*10);
    x[0] = 10;
    if(x == 0) {
        int errsv = errno;
        perror("x is NULL");
        printf("err %d\n", errsv);
        return 0;
    }
    printf("%p\n", x);
    for(int i = 0; i < 10; i++) {
        x[i] = i * i;
    }
    for(int i = 0; i < 10; i++) {
        printf("x[%d]=%d\n",i,x[i]);
    }
    fflush(stdout);
    printf("Enter char: ");
    fflush(stdout);
    // char c = getc(stdin);
    char c = 'a';
    printf("I got: '%c'\n", c);
    return 0;
}
