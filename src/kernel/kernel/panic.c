#include <panic.h>
#include <printf.h>
#include <stdarg.h>
#include <string.h>
#include <util.h>

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
  printf("panic: kernel exception!\n");
  printf("panic: %s\n", exceptions[regs->intId]);
  printf("panic: err %016llx int %016llx\n", regs->errId, regs->intId);
  printf("panic: rax %016llx r8  %016llx\n", regs->rax, regs->r8);
  printf("panic: rbx %016llx r9  %016llx\n", regs->rbx, regs->r9);
  printf("panic: rcx %016llx r10 %016llx\n", regs->rcx, regs->r10);
  printf("panic: rdx %016llx r11 %016llx\n", regs->rdx, regs->r11);
  printf("panic: rsi %016llx r12 %016llx\n", regs->rsi, regs->r12);
  printf("panic: rdi %016llx r13 %016llx\n", regs->rdi, regs->r13);
  printf("panic: rsp %016llx r14 %016llx\n", regs->rsp, regs->r14);
  printf("panic: rip %016llx r15 %016llx\n", regs->rip, regs->r15);
  printf("panic: rfl %016llx\n", regs->rFlags);
  printf("panic: cs  %016llx ss  %016llx\n", regs->cs, regs->ss);
  printf("panic: kRsp %016llx\n", regs->kRsp);
  panic("");
}

void doPanic(char* err) {
  u32 eax = 1, ebx, ecx, edx;
  cpuid(&eax, &ebx, &ecx, &edx);
  u32 cpuId = (ebx >> 24) & 0xFF;

  if (*err != '\0')
    printf("panic: %s", err);
  printf("panic: caused by cpu%d\n", cpuId);
  printf("panic: --- Kernel Call Trace ---\n");

  struct Stacktrace* stk;
  asm("movq %%rbp,%0" : "=r"(stk)::);

  for (u64 fr = 0; stk && fr < 10; ++fr) {
    if (stk->rip == 0)
      break;
    printf("panic: %llx\n", stk->rip);
    stk = stk->rbp;
  }

  printf("panic: call trace end, everything halted!\n");
  disableInts();
  halt();
}

void panic(char* fmt, ...) {
  va_list va;
  va_start(va, fmt);

  char buf[1024];
  memset(buf, 0, sizeof(buf));
  vsnprintf(buf, sizeof(buf), fmt, va);

  doPanic(buf);

  va_end(va);
}