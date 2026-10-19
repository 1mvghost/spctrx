#ifndef PCI_H
#define PCI_H

#include <ll.h>
#include <util.h>

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

void pciWrite32(u32 bus, u32 dev, u32 func, u32 offset, u32 value);
void pciWrite16(u32 bus, u32 dev, u32 func, u32 offset, u16 value);
void pciWrite8(u32 bus, u32 dev, u32 func, u32 offset, u8 value);

u32 pciRead32(u32 bus, u32 dev, u32 func, u32 offset);
u16 pciRead16(u32 bus, u32 dev, u32 func, u32 offset);
u8 pciRead8High(u32 bus, u32 dev, u32 func, u32 offset);
u8 pciRead8Low(u32 bus, u32 dev, u32 func, u32 offset);

void pciInit();

#endif