#include <stdio.h>
#include <stdlib.h>

// FIXME: Currently failing on elf
int main() {
    int a = 11;
    printf("%f\n", 67.88);
    char buf[20];
    gcvt(67.88, 5, buf);
    printf("%s\n", buf);
    // printf("int %d float %f double %f\n", a, (float)a, (double)a);
    // printf("int %d float %f double %f\n", 11, 11.0, 11.0);
    return 0;
}
