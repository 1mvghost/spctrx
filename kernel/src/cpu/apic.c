#include <cpu.h>
#include <ioapic.h>
#include <lapic.h>
#include <panic.h>
#include <util.h>

bool apicCheckSupport() {
  u32 eax = 1, ebx, ecx, edx;
  cpuid(&eax, &ebx, &ecx, &edx);

  return (edx & CPUID_FEAT_EDX_APIC) != 0;
}

void apicInit() {
  if (!apicCheckSupport()) {
    panic("apic is not supported!\n");
  }

  ioApicInit();
  lapicInit();
}