#include <acpi.h>
#include <debug.h>
#include <ioapic.h>
#include <lapic.h>
#include <panic.h>
#include <slab.h>
#include <vmm.h>

static LLHead ioApics;
static LLHead isaOverrides;

static SlabCache ioApicCache;
static SlabCache isaOverrideCache;

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
      ISAOverride* override = slabAlloc(&isaOverrideCache);

      override->bus = *(u8*)(ptr + 2);
      override->isaSource = *(u8*)(ptr + 3);
      override->gsi = *(u32*)(ptr + 4);
      override->flags = *(u16*)(ptr + 8);
      llInitHead(&override->head);

      llInsertFront(&isaOverrides, &override->head);

      debug("ioapic: new override for isa %d -> gsi %d flags %x\n",
            override->isaSource, override->gsi, override->flags);
    }

    ptr += ent->entryLength;
  }
}

void ioApicWriteRedirEntry(IOAPIC* ioApic,
                           int entryId,
                           u8 vector,
                           u8 delivery,
                           u8 destinationMode,
                           u8 deliveryStatus,
                           u8 polarity,
                           u8 trigger,
                           u8 mask,
                           u32 destination) {
  u32 low = 0;
  u32 high = 0;

  low |= vector;
  low |= (delivery & 0b11) << 7;
  low |= (destinationMode & 1) << 10;
  low |= (deliveryStatus & 1) << 11;
  low |= (polarity & 1) << 12;
  low |= (trigger & 1) << 14;
  low |= (mask & 1) << 15;

  high |= (destination << 24);

  int pos = (0x10 + 2 * entryId);

  ioApicWriteReg(ioApic, pos, low);
  ioApicWriteReg(ioApic, pos + 1, high);
}

IOAPIC* ioApicFindForGSI(int gsi) {
  LLHead* curr;

  LL_TRAVERSE(curr, &ioApics) {
    IOAPIC* currIoApic = LIST_ENTRY(curr, IOAPIC, head);

    if (gsi >= currIoApic->gsiStart && gsi < currIoApic->gsiEnd) {
      return currIoApic;
    }
  }
  return 0;
}

void ioApicMap(int gsi, int vector, u16 polarity, u16 trigger) {
  IOAPIC* ioApic = ioApicFindForGSI(gsi);

  if (ioApic == 0) {
    panic("couldn't find an i/o apic for gsi %d!!!\n", gsi);
  }

  ioApicWriteRedirEntry(ioApic, gsi - ioApic->gsiStart, vector, 0, 0, 0,
                        polarity, trigger, 0, 0);
  debug("ioapic: mapped gsi %d to vector %d\n", gsi, vector);
}

void ioApicMapISA(int isa, int vector) {
  int gsi = isa;

  u16 polarity = 0;
  u16 trigger = 0;

  /**
   * check for isa override
   */
  LLHead* curr;

  LL_TRAVERSE(curr, &isaOverrides) {
    ISAOverride* currOverride = LIST_ENTRY(curr, ISAOverride, head);

    if (currOverride->isaSource == isa) {
      gsi = currOverride->gsi;

      polarity = *(u16*)(&currOverride->flags);
      trigger = *(u16*)(&currOverride->flags + 2);

      break;
    }
  }
  ioApicMap(gsi, vector, polarity, trigger);
}

void ioApicInit() {
  slabInitCache(&ioApicCache, "io apics", sizeof(IOAPIC));
  slabInitCache(&isaOverrideCache, "io apic isa overrides",
                sizeof(ISAOverride));

  llInitHead(&ioApics);
  llInitHead(&isaOverrides);

  ioApicParseMadt();
}