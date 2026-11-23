#include <acpi.h>
#include <boot.h>
#include <io.h>
#include <limine.h>
#include <log.h>
#include <paging.h>
#include <panic.h>
#include <pci.h>
#include <pmm.h>
#include <printf.h>
#include <string.h>
#include <vmm.h>

/**
 * TODO: uacpi
 */

#define DEBUG(fmt, ...) dprintf("acpi", fmt, __VA_ARGS__)

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_rsdp_request
    rsdpRequest = {.id = LIMINE_RSDP_REQUEST_ID, .revision = 4};

static RSDP* rsdp;
static RSDT* rsdt;
static FADT* fadt;
static int SLP_TYPa;
static int SLP_TYPb;

void acpiOut(GenericAddress gAddr, u8 val) {
  if (gAddr.addrSpace == 0) {
    vmmMap(TO_HHDM(gAddr.addr), gAddr.addr, PAGE_WRITABLE | PAGE_MMIO,
           PAGE_SIZE);

    if (gAddr.accessSize <= 1)
      *((u8*)TO_HHDM(gAddr.addr)) = val;
    if (gAddr.accessSize == 2)
      *((u16*)TO_HHDM(gAddr.addr)) = val;
    if (gAddr.accessSize == 3)
      *((u32*)TO_HHDM(gAddr.addr)) = val;
    if (gAddr.accessSize == 4)
      *((u64*)TO_HHDM(gAddr.addr)) = val;
  }
  if (gAddr.addrSpace == 1) {
    if (gAddr.accessSize <= 1)
      archOut8((u16)gAddr.addr, val);
    if (gAddr.accessSize == 2)
      archOut16((u16)gAddr.addr, val);
    if (gAddr.accessSize == 3)
      archOut32((u16)gAddr.addr, val);
  }
}
void acpiFadt() {
  fadt = acpiFindTable("FACP");
  if (fadt == 0) {
    return;
  }
  if (!memcmp(TO_HHDM(fadt->Dsdt), "DSDT", 4)) {
    SDTHeader* dsdt = (SDTHeader*)TO_HHDM(fadt->Dsdt);
    char* s5 = (char*)TO_HHDM((fadt->Dsdt + 36));

    bool found = false;

    for (u32 i = 0; i < dsdt->length; i++) {
      if (!memcmp(s5, "_S5", 3)) {
        found = true;
        break;
      }
      s5++;
    }
    if (found) {
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
      DEBUG("FAILED TO FIND S5, SHUTDOWN WONT BE POSSIBLE :(\n", "");
    }
  }
}

void* acpiFindTable(char* signature) {
  for (u32 i = 0; i < (rsdt->h.length - sizeof(rsdt->h)) / 4; i++) {
    void* a = TO_HHDM(rsdt->sdtPtr[i]);
    SDTHeader* h = (SDTHeader*)a;
    if (!memcmp(h->signature, signature, 4)) {
      return a;
    }
  }
  return 0;
}
void acpiRsdt() {
  for (u32 i = 0; i < (rsdt->h.length - sizeof(rsdt->h)) / 4; i++) {
    void* a = TO_HHDM(rsdt->sdtPtr[i]);
    SDTHeader* h = (SDTHeader*)a;
    DEBUG("FOUND TABLE: %c%c%c%c (%x)\n", h->signature[0], h->signature[1],
          h->signature[2], h->signature[3], a);
  }
}
void acpiReboot() {
  acpiOut(fadt->ResetReg, fadt->ResetValue);
}
void acpiShutdown() {
  archOut16(fadt->PM1aControlBlock, SLP_TYPa | 1 << 13);
  if (fadt->PM1bControlBlock != 0) {
    archOut16(fadt->PM1bControlBlock, SLP_TYPb | 1 << 13);
  }
  panic("SHUTDOWN FAILED\n");
}

void acpiInit() {
  if (rsdpRequest.response == 0 || rsdpRequest.response->address == 0) {
    panic("acpi not found!\n");
  }

  rsdp = (RSDP*)rsdpRequest.response->address;
  rsdt = (RSDT*)TO_HHDM(rsdp->rsdt);

  DEBUG("ACPI OEM: %c%c%c%c%c%c\n", rsdp->oemId[0], rsdp->oemId[1],
        rsdp->oemId[2], rsdp->oemId[3], rsdp->oemId[4], rsdp->oemId[5]);
  DEBUG("RSDT ADDR: %x\n", rsdp->rsdt);

  acpiRsdt();

  acpiFadt();
}