#ifndef X86_64_PANIC_H
#define X86_64_PANIC_H

#include <arch/x86_64/isr.h>

struct Stacktrace {
  struct Stacktrace* rbp;
  u64 rip;
};

void panicIsr(Regs* regs);

#endif