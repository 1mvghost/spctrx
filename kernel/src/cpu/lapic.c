#include <io.h>
#include <lapic.h>
#include <printf.h>
#include <util.h>
#include <vmm.h>

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
  lapicWriteReg(LAPIC_REGEOI, 0);
}

void lapicInit() {
  /**
   * disable the pic
   */
  out8(PIC1_DATA, 0xff);
  out8(PIC2_DATA, 0xff);

  vmmMapMMIO(lapicGetBase(), 1);

  /**
   * enable the lapic
   */
  lapicSetBase(lapicGetBase());

  /**
   * set the spurious interrupt register to start receiving interrupts
   */
  lapicWriteReg(LAPIC_REGSPI, lapicReadReg(LAPIC_REGSPI) | 0x100);

  printf("lapic: base %llx regver %x regid %x\n", lapicGetBase(),
         lapicReadReg(LAPIC_REGVER), lapicReadReg(LAPIC_REGID));
}
