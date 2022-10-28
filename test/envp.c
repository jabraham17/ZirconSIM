
#include <elf.h>
#include <stdio.h>

int main(int argc, char** argv, char** envp) {
    // puts("hello\n");
    printf("ARGC %d\n", argc);
    int i;
    printf("ARGS %p\n", argv);
    for(i = 0; i < argc; i++) {
        printf("%p %p %s\n", argv + i, argv[i], argv[i]);
    }
    printf("AFTER ARGS %p %p\n", argv + i, argv[i]);

    printf("ENV %p\n", envp);
    for(i = 0; envp[i] != 0; i++) {
        printf("%p %p %s\n", envp + i, envp[i], envp[i]);
    }
    printf("AFTER ENV %p %p\n", envp + i, envp[i]);

    for(i = 0; envp[i]; i++)
        ;
    long* auxv = (void*)(envp + i + 1);
    for(; auxv[0] != AT_NULL; auxv += 2) {
        printf("%d is: 0x%x\n", auxv[0], auxv[1]);
    }
    return 0;
}
