#include <bitmap.h>
#include <debug.h>
#include <irq.h>
#include <string.h>
#include <vmm.h>

static void (*handlers[256 - 32])();
static Bitmap irqBitmap;

void irqHandle(int irq) {
  handlers[irq]();

  debug("irq: received irq %d\n", irq);
}

int irqAllocVector() {
  int vector = bitmapFind(&irqBitmap, 1);

  bitmapSet(&irqBitmap, vector, true);

  return vector;
}

void irqInstall(int vector, void* handler) {
  handlers[vector - 32] = handler;
}

void irqInit() {
  bitmapInit(&irqBitmap, 256 - 32, vmmAlloc(1));
}