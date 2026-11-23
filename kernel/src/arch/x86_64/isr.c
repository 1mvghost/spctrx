#include <arch/x86_64/idt.h>
#include <arch/x86_64/isr.h>
#include <arch/x86_64/panic.h>
#include <irq.h>

extern u64 isrStub[256];

void isrHandler(Regs* regs) {
  if (regs->intId < 32) {
    panicIsr(regs);
  } else {
    irqHandle(regs->intId - 32);
  }
}

void isrInit() {
  for (int i = 0; i < 256; i++) {
    idtSetDesc(i, (void*)isrStub[i], 0x8E);
  }
}