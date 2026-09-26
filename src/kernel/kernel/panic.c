#include <panic.h>
#include <printf.h>
#include <stdarg.h>
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
  printf("%s\n", exceptions[regs->intId]);
  printf("STOP:%x INT:%x\n", regs->errId, regs->intId);
  printf("RAX:%llx RBX:%llx RCX:%llx RDX:%llx RSP:%llx RDI:%llx RSI:%llx\n",
         regs->rax, regs->rbx, regs->rcx, regs->rdx, regs->rsp, regs->rdi,
         regs->rsi);
  printf(
      "R8:%llx R9:ll%x R10:%llx R11:%llx R12:%llx R13:%llx R14:%llx R15:%llx\n",
      regs->r8, regs->r9, regs->r10, regs->r11, regs->r12, regs->r13, regs->r14,
      regs->r15);
  printf("RIP:%llx CS:%llx RFLAGS:%llx\n", regs->rip, regs->cs, regs->rFlags);
  printf("SS:%llx KRSP:%llx\n", regs->ss, regs->kRsp);
  panic("");
}

void doPanic(char* err) {
  u32 eax = 1, ebx, ecx, edx;
  cpuid(&eax, &ebx, &ecx, &edx);
  u32 cpuId = (ebx >> 24) & 0xFF;

  if (*err != '\0')
    printf("%s", err);
  printf("CPU: %d\n", cpuId);
  printf("--- Kernel Call Trace ---\n");

  struct Stacktrace* stk;
  asm("movq %%rbp,%0" : "=r"(stk)::);

  for (u64 fr = 0; stk && fr < 10; ++fr) {
    if (stk->rip == 0)
      break;
    printf("%llx\n", stk->rip);
    stk = stk->rbp;
  }

  asm("cli");
  asm("hlt");
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