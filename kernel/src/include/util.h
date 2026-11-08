#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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

static inline void disableInts() {
  asm volatile("cli");
}

static inline void enableInts() {
  asm volatile("sti");
}

static inline void halt() {
  asm volatile("hlt");
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