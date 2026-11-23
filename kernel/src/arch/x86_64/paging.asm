[BITS 64]
[GLOBAL pagingInvalidatePage]
[GLOBAL pagingLoad]
[SECTION .text]

pagingInvalidatePage:
    invlpg [rdi]
    ret

pagingLoad:
    mov cr3, rdi
    ret