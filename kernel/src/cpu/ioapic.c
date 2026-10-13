#include <acpi.h>
#include <debug.h>
#include <ioapic.h>
#include <panic.h>
#include <slab.h>
#include <vmm.h>

static LLHead ioApics;
static LLHead ioApicOverrides;

static SlabCache ioApicCache;
static SlabCache ioApicOverrideCache;

void ioApicWriteReg(IOAPIC* ioApic, u32 reg, u32 value) {
  *(u32 volatile*)ioApic->ioAddr = reg;
  *(u32 volatile*)(ioApic->ioAddr + 0x10) = value;
}

u32 ioApicReadReg(IOAPIC* ioApic, u32 reg) {
  *(u32 volatile*)ioApic->ioAddr = reg;
  return *(u32 volatile*)(ioApic->ioAddr + 0x10);
}

void ioApicAdd(int id, u64 ioAddr, int baseGsi) {
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

void ioApicParseMadt() {
  MADT* madt = acpiFindTable("APIC");

  if (madt == 0) {
    panic("no i/o apics are available\n");
  }

  char* tableEnd = (char*)madt + madt->header.length;
  char* ptr = (char*)&madt->entries;

  while (ptr < tableEnd) {
    MADTEntry* ent = (MADTEntry*)ptr;

    if (ent->entryType == 1) {
      /**
       * init io apic
       */
      u8 id = *(u8*)(ptr + 2);
      u32 addr = *(u32*)(ptr + 4);
      u32 gsi = *(u32*)(ptr + 8);

      ioApicAdd(id, addr, gsi);
    } else if (ent->entryType == 2) {
      /**
       * init io apic override (isa -> gsi)
       */
      IOAPICOverride* override = slabAlloc(&ioApicOverrideCache);

      override->bus = *(u8*)(ptr + 2);
      override->isaSource = *(u8*)(ptr + 3);
      override->gsi = *(u32*)(ptr + 4);
      override->flags = *(u16*)(ptr + 8);
      llInitHead(&override->head);

      llInsertFront(&ioApicOverrides, &override->head);

      debug("ioapic: new override for isa %d -> gsi %d flags %x\n",
            override->isaSource, override->gsi, override->flags);
    }

    ptr += ent->entryLength;
  }
}

void ioApicInit() {
  slabInitCache(&ioApicCache, "io apics", sizeof(IOAPIC));
  slabInitCache(&ioApicOverrideCache, "io apic overrides",
                sizeof(IOAPICOverride));

  llInitHead(&ioApics);
  llInitHead(&ioApicOverrides);

  ioApicParseMadt();
}