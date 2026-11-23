#include <io.h>

void archOut8(u16 port, u8 value) {
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void archOut16(u16 port, u16 value) {
  asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void archOut32(u16 port, u32 value) {
  asm volatile("outl %%eax, %%dx" : : "a"(value), "Nd"(port));
}

u8 archIn8(u16 port) {
  u8 result;
  asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port) : "memory");

  return result;
}

u16 archIn16(u16 port) {
  u16 result;
  asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port) : "memory");

  return result;
}

u32 archIn32(u16 port) {
  u32 result;
  asm volatile("inl %%dx, %%eax" : "=a"(result) : "Nd"(port) : "memory");

  return result;
}