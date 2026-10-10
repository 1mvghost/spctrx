#include <bitmap.h>
#include <debug.h>
#include <irq.h>
#include <string.h>
#include <vmm.h>

/**
 * don't count the exceptions and the spurious interrupt
 */
#define IRQ_HANDLERS (256 - 32 - 1)

static void (*handlers[IRQ_HANDLERS])();
static Bitmap irqBitmap;

void irqHandle(int irq) {
  handlers[irq]();

  debug("irq: received irq %d\n", irq);
}

int irqAllocVector() {
  int vector = bitmapFind(&irqBitmap, 1);

  bitmapSet(&irqBitmap, vector, true);

  return vector + 32;
}

void irqInstall(int vector, void* handler) {
  handlers[vector - 32] = handler;
}

void irqInit() {
  bitmapInit(&irqBitmap, IRQ_HANDLERS, vmmAlloc(1));
}