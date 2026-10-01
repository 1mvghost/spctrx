#include <apic.h>
#include <cpu.h>
#include <debug.h>
#include <panic.h>
#include <util.h>
#include <vmm.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100  // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

bool apicCheckSupport() {
  u32 eax = 1, ebx, ecx, edx;
  cpuid(&eax, &ebx, &ecx, &edx);

  return (edx & CPUID_FEAT_EDX_APIC) != 0;
}

void lapicSetBase(u64 base) {
  base |= IA32_APIC_BASE_MSR_ENABLE;

  wrmsr(IA32_APIC_BASE_MSR, base);
}

u64 lapicGetBase() {
  u64 base = rdmsr(IA32_APIC_BASE_MSR);

  /**
   * remove flags
   */
  base &= ~0xfff;

  return base;
}

void lapicWriteReg(u32 reg, u32 value) {
  u32 volatile* ptr = (u32 volatile*)vmmPhysToVirt(lapicGetBase() + reg);
  *ptr = value;
}

u32 lapicReadReg(u32 reg) {
  u32 volatile* ptr = (u32 volatile*)vmmPhysToVirt(lapicGetBase() + reg);
  return *ptr;
}

void apicInit() {
  if (!apicCheckSupport()) {
    panic("apic is not supported!\n");
  }

  vmmMapMMIO(lapicGetBase(), 1);

  lapicSetBase(lapicGetBase());

  lapicWriteReg(0xf0, lapicReadReg(0xf0) | 0x100);

  debug("lapic: base %llx regver %x regid %x\n", lapicGetBase(),
        lapicReadReg(0x30), lapicReadReg(0x20));

  debug("lapic: alive!\n");
}