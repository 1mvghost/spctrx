#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef unsigned long int size_t;

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
static inline void memset(void* dst, u8 value, int n) {
  u8* d = dst;

  while (n-- > 0) {
    *d++ = value;
  }
}
static inline void* memcpy(void* dst, void* src, int n) {
  u8* d = (u8*)dst;
  u8* s = (u8*)src;
  for (int i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dst;
}
static inline void* memmove(void* dstptr, const void* srcptr, u64 size) {
  unsigned char* dst = (unsigned char*)dstptr;
  const unsigned char* src = (const unsigned char*)srcptr;
  if (dst < src) {
    for (u64 i = 0; i < size; i++)
      dst[i] = src[i];
  } else {
    for (u64 i = size; i != 0; i--)
      dst[i - 1] = src[i - 1];
  }
  return dstptr;
}
static inline void sti() {
  __asm__ volatile("sti");
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
static inline int memcmp(void* a, void* b, int cnt) {
  if (!cnt)
    return 0;

  while (--cnt && *(char*)a == *(char*)b) {
    a = (char*)a + 1;
    b = (char*)b + 1;
  }

  return (*((u8*)a) - *((u8*)b));
}
static inline void wrmsr(u64 msr, u64 value) {
  u32 low = value & 0xFFFFFFFF;
  u32 high = value >> 32;
  asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}
static inline u64 rdmsr(u64 msr) {
  u32 low, high;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((u64)high << 32) | low;
}
static inline int strlen(char* s) {
  char* p = s;
  int res = 0;
  while (*p) {
    res++;
    p++;
  }
  return res;
}

static inline void cpuid(u32* a, u32* b, u32* c, u32* d) {
  asm volatile("cpuid" : "=b"(*b), "=c"(*c), "=d"(*d) : "a"(*a));
}

static inline int strcmp(const char* s1, const char* s2) {
  const char* ss1 = s1;
  const char* ss2 = s2;
  while (*ss1 && (*ss1 == *ss2)) {
    ss1++;
    ss2++;
  }
  return *(const unsigned char*)ss1 - *(const unsigned char*)ss2;
}
static inline void strcpy(char* dst, char* src) {
  memcpy(dst, src, strlen(src));
}
#endif