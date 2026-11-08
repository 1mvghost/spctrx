#include <io.h>

void out8(u16 port, u8 value) {
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void out16(u16 port, u16 value) {
  asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void out32(u16 port, u32 value) {
  asm volatile("outl %%eax, %%dx" : : "a"(value), "Nd"(port));
}

u8 in8(u16 port) {
  u8 result;
  asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port) : "memory");

  return result;
}

u16 in16(u16 port) {
  u16 result;
  asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port) : "memory");

  return result;
}

u32 in32(u16 port) {
  u32 result;
  asm volatile("inl %%dx, %%eax" : "=a"(result) : "Nd"(port) : "memory");

  return result;
}

/**
 * TODO: deprecated, remove later
 */
u8 keypress() {
  /* doesnt work on uefi real hardware :( */

  /* clear any old data sitting there */
  while ((in8(0x64) & 1) == 0) {
  }
  in8(0x60);

  while ((in8(0x64) & 1) == 0) {
  }
  return in8(0x60);
}