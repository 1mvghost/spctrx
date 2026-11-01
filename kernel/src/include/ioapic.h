#ifndef IOAPIC_H
#define IOAPIC_H

#include <ll.h>

#define IOAPIC_REGID 0x00
#define IOAPIC_REGVER 0x01
#define IOAPIC_REGPRIORITY 0x02
#define IOAPIC_REGREDIR 0x10

typedef struct {
  char* ioAddr;
  u32 gsiStart;
  u32 gsiEnd;
  int id;

  LLHead head;
} IOAPIC;

typedef struct {
  int bus;
  int isaSource;
  u32 gsi;
  u16 flags;

  LLHead head;
} ISAOverride;

void ioApicMap(u32 gsi, int vector, u16 polarity, u16 trigger);
void ioApicMapISA(int isa, int vector);
void ioApicInit();

#endif