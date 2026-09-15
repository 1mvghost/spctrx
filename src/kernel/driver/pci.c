#include <ahci.h>
#include <alloc.h>
#include <debug.h>
#include <ide.h>
#include <ll.h>
#include <pci.h>

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

typedef struct {
  u16 Vendor;
  u16 Device;
  u16 Cmd;
  u16 Status;
  u8 RevisionId;
  u8 ProgIF;
  u8 Subclass;
  u8 Classcode;
  u8 CacheLineSize;
  u8 LatencyTimer;
  u8 Header;
  u8 Bist;
  u32 Bar0;
  u32 Bar1;
  u32 Bar2;
  u32 Bar3;
  u32 Bar4;
  u32 Bar5;

  LLHead Head;
} PCIDevice;

static char* class[32] = {"Unknown",
                          "Mass Storage Controller",
                          "Network Controller",
                          "Display Controller",
                          "Multimedia Controller",
                          "Memory Controller",
                          "Bridge",
                          "Simple Communication Controller",
                          "Base System Peripheral",
                          "Input Device Controller",
                          "Docking Station",
                          "Processor",
                          "Serial Bus Controller",
                          "Wireless Controller",
                          "Intelligent Controller",
                          "Satellite Communication Controller",
                          "Encryption Controller",
                          "Signal Processing Controller"};

static LLHead pciDevices;

u32 pciIn32(u32 bus, u32 dev, u32 func, u32 offset) {
  u32 address;
  address = (u32)(((u32)0x80000000) | (bus << 16) | (dev << 11) | (func << 8) |
                  (offset & 0xFC));
  out32(CONFIG_ADDRESS, address);
  return in32(CONFIG_DATA);
}

u16 pciIn16(u32 bus, u32 dev, u32 func, u32 offset) {
  /*
  u32 address;
  u16 tmp = 0;
  address = (u32) (((u32)0x80000000) | (bus << 16) | (dev << 11) | (func << 8) |
  (offset & 0xFC)); out32(CONFIG_ADDRESS, address);
  */
  u16 tmp = 0;
  tmp = (u16)((pciIn32(bus, dev, func, offset) >> ((offset & 2) * 8)) & 0xFFFF);
  return tmp;
}
u8 pciIn8High(u32 bus, u32 dev, u32 func, u32 offset) {
  u16 tmp = pciIn16(bus, dev, func, offset);
  return (u8)(tmp >> 8);
}
u8 pciIn8Low(u32 bus, u32 dev, u32 func, u32 offset) {
  u16 tmp = pciIn16(bus, dev, func, offset);
  return (u8)(tmp & 0xFF);
}
void pciHandle(PCIDevice* dev) {
  if (dev->Classcode == 1 && dev->Subclass == 1) {
    /* IDE */
    ideInit(dev->Bar0, dev->Bar1, dev->Bar2, dev->Bar3, dev->Bar4);
  }
  if (dev->Classcode == 1 && dev->Subclass == 6) {
    /* AHCI */
    ahciInit(dev->Bar5);
  }
}

void pciReadData(u32 bus, u32 dev, u32 f, PCIDevice* buf) {
  /*
      u16 device =        pciIn16(bus,dev,f,2);
      u16 cmd =           pciIn16(bus,dev,f,4);
      u16 status =        pciIn16(bus,dev,f,6);
      u8 revisionId =     pciIn8Low(bus,dev,f,8);
      u8 prog =           pciIn8High(bus,dev,f,8);
      u8 subclass =       pciIn8Low(bus,dev,f,10);
      u8 classcode =      pciIn8High(bus,dev,f,10);
      u8 header =         pciIn8Low(bus,dev,f,14);
  */

  buf->Vendor = pciIn16(bus, dev, f, 0);
  buf->Device = pciIn16(bus, dev, f, 2);
  buf->Cmd = pciIn16(bus, dev, f, 4);
  buf->Status = pciIn16(bus, dev, f, 6);
  buf->RevisionId = pciIn8Low(bus, dev, f, 8);
  buf->ProgIF = pciIn8High(bus, dev, f, 8);
  buf->Subclass = pciIn8Low(bus, dev, f, 10);
  buf->Classcode = pciIn8High(bus, dev, f, 10);
  buf->CacheLineSize = pciIn8Low(bus, dev, f, 0xC);
  buf->LatencyTimer = pciIn8High(bus, dev, f, 0xC);
  buf->Header = pciIn8Low(bus, dev, f, 0xF);
  buf->Bist = pciIn8High(bus, dev, f, 0xF);
  buf->Bar0 = pciIn32(bus, dev, f, 0x10);
  buf->Bar1 = pciIn32(bus, dev, f, 0x14);
  buf->Bar2 = pciIn32(bus, dev, f, 0x18);
  buf->Bar3 = pciIn32(bus, dev, f, 0x1C);
  buf->Bar4 = pciIn32(bus, dev, f, 0x20);
  buf->Bar5 = pciIn32(bus, dev, f, 0x24);
}
void pciCheckDevice(u32 bus, u32 dev) {
  for (int f = 0; f < 8; f++) {
    u16 vendor = pciIn16(bus, dev, f, 0);
    /* 0xFFFF - NONEXISTENT DEVICE */
    if (vendor != 0xFFFF) {
      PCIDevice* d = (PCIDevice*)malloc(sizeof(PCIDevice));
      llInitHead(&d->Head);

      pciReadData(bus, dev, f, d);

      llInsertFront(&pciDevices, &d->Head);

      debug(
          "pci: FOUND PCI: %s(%d) VENDOR:%x BAR0:%x BAR1:%x BAR2:%x BAR3:%x "
          "BAR4:%x BAR5:%x HEADER:%x\n",
          class[d->Classcode], d->Classcode, vendor, d->Subclass, d->Bar0,
          d->Bar1, d->Bar2, d->Bar3, d->Bar4, d->Bar5, d->Header);

      pciHandle(d);
    }
  }
}
void pciEnum() {
  for (u32 bus = 0; bus < 256; bus++) {
    for (u32 dev = 0; dev < 32; dev++) {
      pciCheckDevice(bus, dev);
    }
  }
}
void pciInit() {
  llInitHead(&pciDevices);

  pciEnum();
}