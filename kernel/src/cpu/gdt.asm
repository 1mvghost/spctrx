[BITS 64]
[GLOBAL gdtLoad]
[GLOBAL segReload]
[SECTION .text]
gdtLoad:
    lgdt     [rdi]
    ret

segReload:
    push 0x08
    lea rax, [rel .csReload]
    push rax
    retfq

.csReload:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret