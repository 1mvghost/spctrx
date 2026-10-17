#include <debug.h>
#include <idt.h>
#include <irq.h>
#include <isr.h>
#include <panic.h>

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