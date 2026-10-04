#ifndef APIC_H
#define APIC_H

#include <ll.h>

typedef struct {
  void* ioAddr;
  int gsiStart;
  int gsiEnd;
  int id;
  LLHead head;
} IOAPIC;

void apicApInit();
void apicInit();

#endif