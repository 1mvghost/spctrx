#include <debug.h>
#include <lapic.h>
#include <util.h>
#include <vmm.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100  // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

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

void lapicSendEOI() {
  lapicWriteReg(0xb0, 0);
}

void lapicInit() {
  vmmMapMMIO(lapicGetBase(), 1);

  /**
   * enable the lapic
   */
  lapicSetBase(lapicGetBase());

  /**
   * set the spurious interrupt register to start receiving interrupts
   */
  lapicWriteReg(0xf0, lapicReadReg(0xf0) | 0x100);

  debug("lapic: base %llx regver %x regid %x\n", lapicGetBase(),
        lapicReadReg(0x30), lapicReadReg(0x20));
}
