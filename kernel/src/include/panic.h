#ifndef PANIC_H
#define PANIC_H

#include <isr.h>
#include <util.h>

struct Stacktrace {
  struct Stacktrace* rbp;
  u64 rip;
};

void panicIsr(Regs* regs);
void panic(char* fmt, ...);

#endif