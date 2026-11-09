#include <cpu.h>
#include <msr.h>

bool msrCheckSupport() {
  u32 eax = 1, ebx, ecx, edx;
  cpuid(&eax, &ebx, &ecx, &edx);

  return (edx & CPUID_FEAT_EDX_MSR) != 0;
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