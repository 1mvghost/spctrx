#include <bitmap.h>
#include <irq.h>
#include <printf.h>
#include <vmm.h>

/**
 * don't count the exceptions
 */
#define IRQ_HANDLERS (256 - 32)

static void (*handlers[IRQ_HANDLERS])();
static Bitmap irqBitmap;

void irqHandle(int irq) {
  handlers[irq]();

  printf("irq: received irq %d\n", irq);
}

int irqAllocVector() {
  int vector = bitmapFind(&irqBitmap, 1);

  bitmapSet(&irqBitmap, vector, true);

  return vector + 32;
}

void irqInstall(int vector, void (*handler)()) {
  vector -= 32;

  handlers[vector] = handler;

  bitmapSet(&irqBitmap, vector, true);
}

void irqHandleSpurious() {
  printf("irq: spurious interrupt!\n");
}

void irqInit() {
  bitmapInit(&irqBitmap, IRQ_HANDLERS, vmmAlloc(1));

  irqInstall(255, irqHandleSpurious);
}