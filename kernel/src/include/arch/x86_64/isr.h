#ifndef X86_64_ISR_H
#define X86_64_ISR_H

#include <util.h>

typedef struct {
  u64 rdiIgnore;
  u64 r15, r14, r13, r12, r11, r10, r9, r8, rsp, rsi, rdi, rdx, rcx, rbx, rax;
  u64 intId, errId;
  u64 rip, cs, rFlags, kRsp, ss;
} __attribute__((packed)) Regs;

void isrInit();

#endif