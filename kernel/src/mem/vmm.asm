[BITS 64]
[GLOBAL vmmLoad]
[SECTION .text]

vmmLoad:
    mov cr3, rdi
    ret