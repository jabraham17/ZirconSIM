
.section data
.global elf_symbol
elf_symbol:
.word 0xDEADBEEF

.section text
.global _start
_start:
    li a0, 0xFFFFFFFFFFFFFFFF
    1: j 1b
# We will have a state file to assert a0 has the correct value
