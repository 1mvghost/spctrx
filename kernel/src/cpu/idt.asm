[BITS 64]
[GLOBAL idtLoad]
[SECTION .text]
idtLoad:
    lidt     [rdi]
    ret