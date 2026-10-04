#include <acpi.h>
#include <apic.h>
#include <cpu.h>
#include <debug.h>
#include <panic.h>
#include <slab.h>
#include <util.h>
#include <vmm.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100  // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

static LLHead ioApics;
static SlabCache ioApicCache;

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

void ioApicWriteReg(IOAPIC* ioApic, u32 reg, u32 value) {
  *(u32 volatile*)ioApic->ioAddr = reg;
  *(u32 volatile*)(ioApic->ioAddr + 0x10) = value;
}

u32 ioApicReadReg(IOAPIC* ioApic, u32 reg) {
  *(u32 volatile*)ioApic->ioAddr = reg;
  return *(u32 volatile*)(ioApic->ioAddr + 0x10);
}

void apicInitIoApic(int id, u64 ioAddr, int baseGsi) {
  IOAPIC* ioApic = slabAlloc(&ioApicCache);

  vmmMapMMIO(ioAddr, 1);

  ioApic->ioAddr = vmmPhysToVirt(ioAddr);
  ioApic->gsiStart = baseGsi;

  int redirEntries = (ioApicReadReg(ioApic, 0x01) >> 16) + 1;
  ioApic->gsiEnd = baseGsi + redirEntries;

  ioApic->id = id;

  llInitHead(&ioApic->head);
  llInsertFront(&ioApics, &ioApic->head);

  debug("ioapic: new io apic ioaddr %llx gsistart %d gsiend %d\n",
        ioApic->ioAddr, ioApic->gsiStart, ioApic->gsiEnd);
}

void apicFindIoApics() {
  /**
   * todo: check for io apic overrides
   */
  MADT* madt = acpiFindTable("APIC");

  if (madt == 0) {
    panic("no i/o apics are available\n");
  }

  char* tableEnd = (char*)madt + madt->header.length;
  char* ptr = (char*)&madt->entries;

  while (ptr < tableEnd) {
    MADTEntry* ent = (MADTEntry*)ptr;

    if (ent->entryType == 1) {
      u8 id = *(u8*)(ptr + 2);
      u32 addr = *(u32*)(ptr + 4);
      u32 gsi = *(u32*)(ptr + 8);

      apicInitIoApic(id, addr, gsi);
    }

    ptr += ent->entryLength;
  }
}

void apicApInit() {
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

void apicInit() {
  if (!apicCheckSupport()) {
    panic("apic is not supported!\n");
  }

  slabInitCache(&ioApicCache, "io apic object cache", sizeof(IOAPIC));
  llInitHead(&ioApics);

  apicApInit();
  apicFindIoApics();
}