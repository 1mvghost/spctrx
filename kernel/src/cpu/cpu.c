#include <cpu.h>

void disableInts() {
  asm volatile("cli");
}

void enableInts() {
  asm volatile("sti");
}

void waitForInterrupt() {
  asm volatile("hlt");
}

void halt() {
  disableInts();

  for (;;) {
    waitForInterrupt();
  }
}

void wrmsr(u32 msr, u64 value) {
  u32 low = value & 0xFFFFFFFF;
  u32 high = value >> 32;

  asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high) : "memory");
}

u64 rdmsr(u32 msr) {
  u32 low, high;

  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

  return ((u64)high << 32) | low;
}

void cpuid(u32* eax, u32* ebx, u32* ecx, u32* edx) {
  asm volatile("cpuid" : "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(*eax));
}