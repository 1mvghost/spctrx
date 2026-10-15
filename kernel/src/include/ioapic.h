#ifndef IOAPIC_H
#define IOAPIC_H

#include <ll.h>

typedef struct {
  void* ioAddr;
  int gsiStart;
  int gsiEnd;
  int id;

  LLHead head;
} IOAPIC;

typedef struct {
  int bus;
  int isaSource;
  int gsi;
  int flags;

  LLHead head;
} ISAOverride;

void ioApicMap(int gsi, int vector, u16 polarity, u16 trigger);
void ioApicMapISA(int isa, int vector);
void ioApicInit();

#endif