#ifndef IOAPIC_H
#define IOAPIC_H

#include <ll.h>

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