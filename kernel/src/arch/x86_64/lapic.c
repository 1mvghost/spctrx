#include <arch/x86_64/lapic.h>
#include <arch/x86_64/msr.h>
#include <io.h>
#include <log.h>
#include <paging.h>
#include <util.h>
#include <vmm.h>

#define DEBUG(fmt, ...) dprintf("lapic", fmt, __VA_ARGS__)

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
  u32 volatile* ptr = (u32 volatile*)TO_HHDM(lapicGetBase() + reg);
  *ptr = value;
}

u32 lapicReadReg(u32 reg) {
  u32 volatile* ptr = (u32 volatile*)TO_HHDM(lapicGetBase() + reg);
  return *ptr;
}

void lapicSendEOI() {
  lapicWriteReg(LAPIC_REGEOI, 0);
}

void lapicInit() {
  /**
   * disable the pic
   */
  archOut8(PIC1_DATA, 0xff);
  archOut8(PIC2_DATA, 0xff);

  vmmMap(TO_HHDM(lapicGetBase()), lapicGetBase(), PAGE_WRITABLE | PAGE_MMIO,
         PAGE_SIZE);

  /**
   * enable the lapic
   */
  lapicSetBase(lapicGetBase());

  /**
   * set the spurious interrupt register to start receiving interrupts
   */
  lapicWriteReg(LAPIC_REGSPI, lapicReadReg(LAPIC_REGSPI) | 0x100);

  DEBUG("base %llx regver %x regid %x\n", lapicGetBase(),
        lapicReadReg(LAPIC_REGVER), lapicReadReg(LAPIC_REGID));
}
