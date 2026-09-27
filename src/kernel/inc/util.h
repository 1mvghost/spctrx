#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint8_t bool;
#define true 1
#define false 0

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define U64_LOW(addr) (addr & 0xffffffff)
#define U64_HIGH(addr) ((addr >> 32) & 0xffffffff)

#define U64(low, high) (((u64)high << 32) + low)

/* credit:
 * https://github.com/embeddedartistry/libmemory/blob/master/src/aligned_malloc.c
 */
#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

#define ALIGN_DOWN(num, align) (num & ~((align) - 1))

#define UNUSED(param) (void)param

static inline void out8(u16 port, u8 val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline void out16(u16 port, u16 val) {
  __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline void out32(u16 port, u32 val) {
  __asm__ volatile("outl %%eax, %%dx" : : "a"(val), "Nd"(port));
}
static inline u8 in8(u16 port) {
  u8 ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}
static inline u16 in16(u16 port) {
  u16 ret;
  __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}
static inline u32 in32(u16 port) {
  u32 ret;
  __asm__ volatile("inl %%dx, %%eax" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}
static inline void ins32(u16 port, u32* buf, int q) {
  int i = 0;
  for (i = 0; i < q; i++) {
    buf[i] = in32(port);
  }
}
static inline u8 keypress() {
  /* doesnt work on uefi real hardware :( */

  /* clear any old data sitting there */
  while ((in8(0x64) & 1) == 0) {
  }
  in8(0x60);

  while ((in8(0x64) & 1) == 0) {
  }
  return in8(0x60);
}

static inline void wrmsr(u32 msr, u64 value) {
  u32 low = value & 0xFFFFFFFF;
  u32 high = value >> 32;
  asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high) : "memory");
}
static inline u64 rdmsr(u32 msr) {
  u32 low, high;

  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

  return ((u64)high << 32) | low;
}
static inline void cpuid(u32* a, u32* b, u32* c, u32* d) {
  asm volatile("cpuid" : "=b"(*b), "=c"(*c), "=d"(*d) : "a"(*a));
}
#endif