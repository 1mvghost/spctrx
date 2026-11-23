#include <arch/x86_64/cpuid.h>

void cpuid(u32* eax, u32* ebx, u32* ecx, u32* edx) {
  asm volatile("cpuid" : "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(*eax));
}