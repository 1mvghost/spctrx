#include <debug.h>
#include <idt.h>
#include <irq.h>
#include <isr.h>
#include <mem.h>
#include <panic.h>
#include <printf.h>

extern u64 isrStub[256];

void isrHandler(Regs* regs) {
  if (regs->intId < 32) {
    panicIsr(regs);
  } else {
    irqHandle(regs->intId - 32);
  }
}

void isrSpuriousInt() {
  debug("isr: spurious interrupt!\n");
}

void isrInit() {
  for (int i = 0; i < 255; i++) {
    idtSetDesc(i, (void*)isrStub[i], 0x8E);
  }

  /**
   * spurious interrupt handler
   */
  idtSetDesc(255, isrSpuriousInt, 0x8E);

  enableInts();
}