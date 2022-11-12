
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

uint64_t get_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ull + tv.tv_usec;
}

int64_t dot(int64_t* x, int64_t* y, size_t n) {
    int64_t sum = 0;
    for(size_t i = 0; i < n; i++) {
        sum += x[i] * y[i];
    }
    return sum;
}

#define TIMEIT(iter, avg, var, res, expr)                                      \
    {                                                                          \
        res = 0;                                                               \
        uint64_t sum = 0;                                            \
        uint64_t sum2 = 0;                                           \
        for(size_t i = 0; i < iter; i++) {                                     \
            uint64_t start = get_us();                                         \
            res += expr;                                                       \
            uint64_t stop = get_us();                                          \
            uint64_t t = stop - start;                                         \
            sum += t;                                                          \
            sum2 += t * t;                                                     \
        }                                                                      \
        avg = sum / iter;                                                      \
        var = (sum2 / iter) - (avg * avg);                                     \
    }                                                                          \
    while(0)                                                                   \
        ;

int main() {

    size_t n = 1024 * 4; // each array is a 1/4 page
    size_t iter = 500;
    srand(0xDEADBEEF);
    int64_t* x = malloc(sizeof(*x)*n);
    int64_t* y = malloc(sizeof(*y)*n);
    for(size_t i = 0; i < n; i++) {
        x[i] = 1 + rand() / (1 + rand() % 200);
        y[i] = 1 + rand() / (1 + rand() % 200);
    }

    uint64_t avg;
    uint64_t var;
    volatile int64_t res = 0;

    TIMEIT(iter, avg, var, res, dot(x, y, n))
    printf("RUN\nRES: %lu\nAVG: %lu\nVAR: %lu\n", res, avg, var);
}
