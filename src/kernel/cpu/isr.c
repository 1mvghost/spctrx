#include <debug.h>
#include <idt.h>
#include <isr.h>
#include <mem.h>
#include <panic.h>
#include <printf.h>

extern void isr0(Regs*);
extern void isr1(Regs*);
extern void isr2(Regs*);
extern void isr3(Regs*);
extern void isr4(Regs*);
extern void isr5(Regs*);
extern void isr6(Regs*);
extern void isr7(Regs*);
extern void isr8(Regs*);
extern void isr9(Regs*);
extern void isr10(Regs*);
extern void isr11(Regs*);
extern void isr12(Regs*);
extern void isr13(Regs*);
extern void isr14(Regs*);
extern void isr15(Regs*);
extern void isr16(Regs*);
extern void isr17(Regs*);
extern void isr18(Regs*);
extern void isr19(Regs*);
extern void isr20(Regs*);
extern void isr21(Regs*);
extern void isr22(Regs*);
extern void isr23(Regs*);
extern void isr24(Regs*);
extern void isr25(Regs*);
extern void isr26(Regs*);
extern void isr27(Regs*);
extern void isr28(Regs*);
extern void isr29(Regs*);
extern void isr30(Regs*);
extern void isr31(Regs*);

static void (*isrStub[32])(Regs*) = {
    isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,  isr8,  isr9,  isr10,
    isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20, isr21,
    isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31};

void isrHandler(Regs* regs) {
  panicIsr(regs);
}

void isrSpuriousInt() {
  debug("isr: spurious interrupt!\n");
}

void isrInit() {
  for (int i = 0; i < 32; i++) {
    idtSetDesc(i, (void*)isrStub[i], 0x8E);
  }

  /**
   * spurious interrupt handler
   */
  idtSetDesc(255, isrSpuriousInt, 0x8E);

  enableInts();
}