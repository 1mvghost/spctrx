#include <acpi.h>
#include <boot.h>
#include <debug.h>
#include <limine.h>
#include <panic.h>
#include <pci.h>
#include <pmm.h>
#include <string.h>
#include <vmm.h>

/**
 * TODO: uacpi
 */

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_rsdp_request
    rsdpRequest = {.id = LIMINE_RSDP_REQUEST_ID, .revision = 4};

typedef struct {
  char signature[4];
  u32 length;
  u8 revision;
  u8 checksum;
  char oemId[6];
  char oemTableId[8];
  u32 oemRevision;
  u32 creatorId;
  u32 creatorRevision;
} __attribute__((packed)) SDTHeader;

typedef struct {
  SDTHeader h;
  u32 sdtPtr[256];
} __attribute__((packed)) RSDT;

typedef struct {
  u8 addrSpace;
  u8 bitWidth;
  u8 bitOffset;
  u8 accessSize;
  u64 addr;
} __attribute__((packed)) GenericAddress;

typedef struct {
  SDTHeader h;
  u32 FirmwareCtrl;
  u32 Dsdt;
  // field used in ACPI 1.0; no longer in use, for compatibility only
  u8 Reserved;
  u8 PreferredPowerManagementProfile;
  u16 SCI_Interrupt;
  u32 SMI_CommandPort;
  u8 AcpiEnable;
  u8 AcpiDisable;
  u8 S4BIOS_REQ;
  u8 PSTATE_Control;
  u32 PM1aEventBlock;
  u32 PM1bEventBlock;
  u32 PM1aControlBlock;
  u32 PM1bControlBlock;
  u32 PM2ControlBlock;
  u32 PMTimerBlock;
  u32 GPE0Block;
  u32 GPE1Block;
  u8 PM1EventLength;
  u8 PM1ControlLength;
  u8 PM2ControlLength;
  u8 PMTimerLength;
  u8 GPE0Length;
  u8 GPE1Length;
  u8 GPE1Base;
  u8 CStateControl;
  u16 WorstC2Latency;
  u16 WorstC3Latency;
  u16 FlushSize;
  u16 FlushStride;
  u8 DutyOffset;
  u8 DutyWidth;
  u8 DayAlarm;
  u8 MonthAlarm;
  u8 Century;
  // reserved in ACPI 1.0; used since ACPI 2.0+
  u16 BootArchitectureFlags;
  u8 Reserved2;
  u32 Flags;
  // 12 byte structure; see below for details
  GenericAddress ResetReg;
  u8 ResetValue;
  u8 Reserved3[3];

  // 64bit pointers - Available on ACPI 2.0+
  u64 X_FirmwareControl;
  u64 X_Dsdt;
  GenericAddress X_PM1aEventBlock;
  GenericAddress X_PM1bEventBlock;
  GenericAddress X_PM1aControlBlock;
  GenericAddress X_PM1bControlBlock;
  GenericAddress X_PM2ControlBlock;
  GenericAddress X_PMTimerBlock;
  GenericAddress X_GPE0Block;
  GenericAddress X_GPE1Block;
} __attribute__((packed)) FADT;

static RSDP* rsdp;
static RSDT* rsdt;
static FADT* fadt;
static int SLP_TYPa;
static int SLP_TYPb;

void acpiOut(GenericAddress gAddr, u8 val) {
  if (gAddr.addrSpace == 0) {
    if (gAddr.accessSize <= 1)
      *((u8*)vmmPhysToVirt(gAddr.addr)) = val;
    if (gAddr.accessSize == 2)
      *((u16*)vmmPhysToVirt(gAddr.addr)) = val;
    if (gAddr.accessSize == 3)
      *((u32*)vmmPhysToVirt(gAddr.addr)) = val;
    if (gAddr.accessSize == 4)
      *((u64*)vmmPhysToVirt(gAddr.addr)) = val;
  }
  if (gAddr.addrSpace == 1) {
    if (gAddr.accessSize <= 1)
      out8((u16)gAddr.addr, val);
    if (gAddr.accessSize == 2)
      out16((u16)gAddr.addr, val);
    if (gAddr.accessSize == 3)
      out32((u16)gAddr.addr, val);
  }
}
void acpiFadt() {
  if (!memcmp(vmmPhysToVirt(fadt->Dsdt), "DSDT", 4)) {
    char* s5 = (char*)vmmPhysToVirt((fadt->Dsdt + 36));
    int* len = (int*)vmmPhysToVirt(((fadt->Dsdt + 1) - 36));
    while (len-- > 0) {
      if (!memcmp(s5, "_S5_", 3)) {
        break;
      }
      s5++;
    }
    if (len > 0) {
      s5 += 5;
      s5 += ((*s5 & 0xc0) >> 6) + 2;
      if (*s5 == 0x0A)
        s5++;
      SLP_TYPa = *(s5) << 10;
      s5++;
      if (*s5 == 0x0A)
        s5++;
      SLP_TYPb = *(s5) << 10;
    } else {
      debug("acpi: FAILED TO FIND S5, SHUTDOWN WONT BE POSSIBLE :(\n");
    }
  }
}

void acpiRsdt() {
  for (u32 i = 0; i < (rsdt->h.length - sizeof(rsdt->h)) / 4; i++) {
    void* a = vmmPhysToVirt(rsdt->sdtPtr[i]);
    SDTHeader* h = (SDTHeader*)a;
    debug("acpi: FOUND TABLE: %c%c%c%c (%x)\n", h->signature[0],
          h->signature[1], h->signature[2], h->signature[3], a);
    if (!memcmp(h->signature, "FACP", 4)) {
      fadt = (FADT*)a;
      acpiFadt();
    }
  }
}
void acpiReboot() {
  acpiOut(fadt->ResetReg, fadt->ResetValue);
}
void acpiShutdown() {
  disableInts();

  out16(fadt->PM1aControlBlock, SLP_TYPa | 1 << 13);
  if (fadt->PM1bControlBlock != 0) {
    out16(fadt->PM1bControlBlock, SLP_TYPb | 1 << 13);
  }
  panic("SHUTDOWN FAILED\n");
}

void acpiInit() {
  if (rsdpRequest.response == 0 || rsdpRequest.response->address == 0) {
    panic("acpi not found!\n");
  }

  rsdp = (RSDP*)rsdpRequest.response->address;
  rsdt = (RSDT*)vmmPhysToVirt(rsdp->rsdt);

  debug("acpi: ACPI OEM: %c%c%c%c%c%c\n", rsdp->oemId[0], rsdp->oemId[1],
        rsdp->oemId[2], rsdp->oemId[3], rsdp->oemId[4], rsdp->oemId[5]);
  debug("acpi: RSDT ADDR: %x\n", rsdp->rsdt);

  acpiRsdt();
}