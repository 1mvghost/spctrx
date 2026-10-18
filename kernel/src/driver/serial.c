#include <printf.h>
#include <serial.h>
#include <spinlock.h>
#include <stdarg.h>
#include <string.h>

#define COM1 0x3F8

int serialIsEmpty() {
  return in8(COM1 + 5) & 0x20;
}

static SPINLOCK(serialSpinlock);
void serialPutc(char ch) {
  spinlockAcquire(&serialSpinlock);

  while (serialIsEmpty() == 0) {
  }

  out8(COM1, ch);

  spinlockDrop(&serialSpinlock);
}

void serialInit() {
  out8(COM1 + 1, 0x00);
  out8(COM1 + 3, 0x80);
  out8(COM1 + 0, 0x03);
  out8(COM1 + 1, 0x00);
  out8(COM1 + 3, 0x03);
  out8(COM1 + 2, 0xC7);
  out8(COM1 + 4, 0x0B);
  out8(COM1 + 4, 0x1E);
  out8(COM1 + 0, 0xAE);

  if (in8(COM1 + 0) != 0xAE) {
    return;
  }
  out8(COM1 + 4, 0x0F);
}