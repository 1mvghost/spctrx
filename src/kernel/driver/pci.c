#include <ahci.h>
#include <debug.h>
#include <ide.h>
#include <ll.h>
#include <pci.h>
#include <slab.h>

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

typedef struct {
  u16 vendor;
  u16 device;
  u16 cmd;
  u16 status;
  u8 revisionId;
  u8 progIF;
  u8 subclass;
  u8 classcode;
  u8 cacheLineSize;
  u8 latencyTimer;
  u8 header;
  u8 bist;
  u32 bar0;
  u32 bar1;
  u32 bar2;
  u32 bar3;
  u32 bar4;
  u32 bar5;

  LLHead head;
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

static SlabCache pciDeviceCache;
static LLHead pciDeviceList;

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
  if (dev->classcode == 1 && dev->subclass == 1) {
    /* IDE */
    ideInit(dev->bar0, dev->bar1, dev->bar2, dev->bar3, dev->bar4);
  }
  if (dev->classcode == 1 && dev->subclass == 6) {
    /* AHCI */
    ahciInit(dev->bar5);
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

  buf->vendor = pciIn16(bus, dev, f, 0);
  buf->device = pciIn16(bus, dev, f, 2);
  buf->cmd = pciIn16(bus, dev, f, 4);
  buf->status = pciIn16(bus, dev, f, 6);
  buf->revisionId = pciIn8Low(bus, dev, f, 8);
  buf->progIF = pciIn8High(bus, dev, f, 8);
  buf->subclass = pciIn8Low(bus, dev, f, 10);
  buf->classcode = pciIn8High(bus, dev, f, 10);
  buf->cacheLineSize = pciIn8Low(bus, dev, f, 0xC);
  buf->latencyTimer = pciIn8High(bus, dev, f, 0xC);
  buf->header = pciIn8Low(bus, dev, f, 0xF);
  buf->bist = pciIn8High(bus, dev, f, 0xF);
  buf->bar0 = pciIn32(bus, dev, f, 0x10);
  buf->bar1 = pciIn32(bus, dev, f, 0x14);
  buf->bar2 = pciIn32(bus, dev, f, 0x18);
  buf->bar3 = pciIn32(bus, dev, f, 0x1C);
  buf->bar4 = pciIn32(bus, dev, f, 0x20);
  buf->bar5 = pciIn32(bus, dev, f, 0x24);
}
void pciCheckDevice(u32 bus, u32 dev) {
  for (int f = 0; f < 8; f++) {
    u16 vendor = pciIn16(bus, dev, f, 0);
    /* 0xFFFF - NONEXISTENT DEVICE */
    if (vendor != 0xFFFF) {
      PCIDevice* d = slabAlloc(&pciDeviceCache);

      llInitHead(&d->head);

      pciReadData(bus, dev, f, d);

      llInsertFront(&pciDeviceList, &d->head);

      debug(
          "pci: FOUND PCI: %s(%d) VENDOR:%x BAR0:%x BAR1:%x BAR2:%x BAR3:%x "
          "BAR4:%x BAR5:%x HEADER:%x\n",
          class[d->classcode], d->classcode, vendor, d->subclass, d->bar0,
          d->bar1, d->bar2, d->bar3, d->bar4, d->bar5, d->header);

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
  slabInitCache(&pciDeviceCache, "pci device object cache", sizeof(PCIDevice));

  llInitHead(&pciDeviceList);

  pciEnum();
}