#include <ahci.h>
#include <pci.h>
#include <printf.h>
#include <slab.h>

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
static LLHead pciDevices;

u32 pciGetConfigAddress(u32 bus, u32 dev, u32 func, u32 offset) {
  return ((0x80000000) | (bus << 16) | (dev << 11) | (func << 8) |
          (offset & 0xFC));
}

void pciWrite32(u32 bus, u32 dev, u32 func, u32 offset, u32 value) {
  out32(CONFIG_ADDRESS, pciGetConfigAddress(bus, dev, func, offset));
  out32(CONFIG_DATA, value);
}

void pciWrite16(u32 bus, u32 dev, u32 func, u32 offset, u16 value) {
  out32(CONFIG_ADDRESS, pciGetConfigAddress(bus, dev, func, offset));
  out16(CONFIG_DATA, value);
}

void pciWrite8(u32 bus, u32 dev, u32 func, u32 offset, u8 value) {
  out32(CONFIG_ADDRESS, pciGetConfigAddress(bus, dev, func, offset));
  out8(CONFIG_DATA, value);
}

u32 pciRead32(u32 bus, u32 dev, u32 func, u32 offset) {
  out32(CONFIG_ADDRESS, pciGetConfigAddress(bus, dev, func, offset));
  return in32(CONFIG_DATA);
}

u16 pciRead16(u32 bus, u32 dev, u32 func, u32 offset) {
  return (u16)((pciRead32(bus, dev, func, offset) >> ((offset & 2) * 8)) &
               0xFFFF);
}

u8 pciRead8High(u32 bus, u32 dev, u32 func, u32 offset) {
  u16 tmp = pciRead16(bus, dev, func, offset);
  return (u8)(tmp >> 8);
}

u8 pciRead8Low(u32 bus, u32 dev, u32 func, u32 offset) {
  u16 tmp = pciRead16(bus, dev, func, offset);
  return (u8)(tmp & 0xFF);
}

void pciInitDriver(PCIDevice* dev) {
  if (dev->classcode == 1 && dev->subclass == 6) {
    ahciInit(dev->bar5);
  }
}

void pciReadDeviceData(u32 bus, u32 dev, u32 func, PCIDevice* buf) {
  buf->vendor = pciRead16(bus, dev, func, 0);
  buf->device = pciRead16(bus, dev, func, 2);
  buf->cmd = pciRead16(bus, dev, func, 4);
  buf->status = pciRead16(bus, dev, func, 6);
  buf->revisionId = pciRead8Low(bus, dev, func, 8);
  buf->progIF = pciRead8High(bus, dev, func, 8);
  buf->subclass = pciRead8Low(bus, dev, func, 10);
  buf->classcode = pciRead8High(bus, dev, func, 10);
  buf->cacheLineSize = pciRead8Low(bus, dev, func, 0xC);
  buf->latencyTimer = pciRead8High(bus, dev, func, 0xC);
  buf->header = pciRead8Low(bus, dev, func, 0xF);
  buf->bist = pciRead8High(bus, dev, func, 0xF);
  buf->bar0 = pciRead32(bus, dev, func, 0x10);
  buf->bar1 = pciRead32(bus, dev, func, 0x14);
  buf->bar2 = pciRead32(bus, dev, func, 0x18);
  buf->bar3 = pciRead32(bus, dev, func, 0x1C);
  buf->bar4 = pciRead32(bus, dev, func, 0x20);
  buf->bar5 = pciRead32(bus, dev, func, 0x24);
}

void pciCheckDevice(u32 busId, u32 devId) {
  for (int func = 0; func < 8; func++) {
    u16 vendor = pciRead16(busId, devId, func, 0);

    /**
     * vendor 0xffff - nonexistent device
     */
    if (vendor != 0xFFFF) {
      PCIDevice* dev = slabAlloc(&pciDeviceCache);

      llInitHead(&dev->head);
      pciReadDeviceData(busId, devId, func, dev);

      printf("pci: %02d:%02d.%02d subclass %02x vendor %04x [%s]\n", busId,
             devId, func, dev->subclass, dev->vendor, class[dev->classcode]);

      llInsertFront(&pciDevices, &dev->head);

      pciInitDriver(dev);
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
  slabInitCache(&pciDeviceCache, "pci devices", sizeof(PCIDevice));
  llInitHead(&pciDevices);

  pciEnum();
}