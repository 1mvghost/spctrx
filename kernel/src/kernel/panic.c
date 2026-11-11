#include <cpu.h>
#include <log.h>
#include <panic.h>
#include <printf.h>
#include <stdarg.h>
#include <string.h>
#include <util.h>

#define DEBUG(fmt, ...) dprintf("panic", fmt, __VA_ARGS__)

static const char* exceptions[32] = {"Div By Zero",
                                     "Debug",
                                     "NMI",
                                     "Breakpoint",
                                     "Overflow",
                                     "Bound Range Exceeded",
                                     "Invalid Opcode",
                                     "Device Not Available",
                                     "Double Fault",
                                     "Coprocessor Segment Overrun",
                                     "Bad TSS",
                                     "Segment Not Present",
                                     "Stack-Segment Fault",
                                     "General Protection Fault",
                                     "Page Fault",
                                     "Unknown",
                                     "x87 Floating-Point",
                                     "Alignment Check",
                                     "Machine Check",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown",
                                     "Unknown"};

void panicIsr(Regs* regs) {
  DEBUG("kernel exception!\n", "");
  DEBUG("%s\n", exceptions[regs->intId]);
  DEBUG("err %016llx int %016llx\n", regs->errId, regs->intId);
  DEBUG("rax %016llx r8  %016llx\n", regs->rax, regs->r8);
  DEBUG("rbx %016llx r9  %016llx\n", regs->rbx, regs->r9);
  DEBUG("rcx %016llx r10 %016llx\n", regs->rcx, regs->r10);
  DEBUG("rdx %016llx r11 %016llx\n", regs->rdx, regs->r11);
  DEBUG("rsi %016llx r12 %016llx\n", regs->rsi, regs->r12);
  DEBUG("rdi %016llx r13 %016llx\n", regs->rdi, regs->r13);
  DEBUG("rsp %016llx r14 %016llx\n", regs->rsp, regs->r14);
  DEBUG("rip %016llx r15 %016llx\n", regs->rip, regs->r15);
  DEBUG("rfl %016llx\n", regs->rFlags);
  DEBUG("cs  %016llx ss  %016llx\n", regs->cs, regs->ss);
  DEBUG("kRsp %016llx\n", regs->kRsp);
  panic("");
}

void doPanic(char* err) {
  if (*err != '\0') {
    DEBUG("%s", err);
  }

  DEBUG("--- Kernel Call Trace ---\n", "");

  struct Stacktrace* stk;
  asm("movq %%rbp,%0" : "=r"(stk)::);

  for (u64 fr = 0; stk && fr < 10; ++fr) {
    if (stk->rip == 0) {
      break;
    }
    DEBUG("%llx\n", stk->rip);
    stk = stk->rbp;
  }

  DEBUG("call trace end, everything halted!\n", "");

  halt();
}

void panic(char* fmt, ...) {
  disableInts();

  va_list va;
  va_start(va, fmt);

  char buf[1024];
  memset(buf, 0, sizeof(buf));
  vsnprintf(buf, sizeof(buf), fmt, va);

  doPanic(buf);

  va_end(va);
}