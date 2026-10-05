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

typedef struct {
  int bus;
  int isaSource;
  int gsi;
  int flags;

  LLHead head;
} IOAPICOverride;

void apicApInit();
void apicInit();

#endif