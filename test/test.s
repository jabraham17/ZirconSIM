
# ./toolchains/rv64ima/bin/riscv64-unknown-elf-gcc -c test/test.s -o test/test.o && ./toolchains/rv64ima/bin/riscv64-unknown-elf-ld test/test.o -o test/test.out
.global _start
_start:
    # addi sp, sp, -16
    li a0, 0xFFFFFFFFFFFFFFFF
    li a1, 0xaaaaaaaaaaaaaaaa
    li a2, 0x1111111111111111
    sd a0, -8(sp)
    sd a1, (sp)
    # sh a2, 8(sp)

    ld a3, -8(sp)
    ld a4, (sp)
    # ld a5, 8(sp)

    1: j 1b
