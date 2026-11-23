#include <arch/x86_64/serial.h>
#include <io.h>
#include <spinlock.h>

#define COM1 0x3F8

int serialIsEmpty() {
  return archIn8(COM1 + 5) & 0x20;
}

static SPINLOCK(serialSpinlock);
void archDebugPutc(char ch) {
  spinlockAcquire(&serialSpinlock);

  while (serialIsEmpty() == 0) {
  }

  archOut8(COM1, ch);

  spinlockDrop(&serialSpinlock);
}

void serialInit() {
  archOut8(COM1 + 1, 0x00);
  archOut8(COM1 + 3, 0x80);
  archOut8(COM1 + 0, 0x03);
  archOut8(COM1 + 1, 0x00);
  archOut8(COM1 + 3, 0x03);
  archOut8(COM1 + 2, 0xC7);
  archOut8(COM1 + 4, 0x0B);
  archOut8(COM1 + 4, 0x1E);
  archOut8(COM1 + 0, 0xAE);

  if (archIn8(COM1 + 0) != 0xAE) {
    return;
  }
  archOut8(COM1 + 4, 0x0F);
}