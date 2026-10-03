#ifndef ACPI_H
#define ACPI_H

#include <util.h>

typedef struct {
  char signature[8];
  u8 checksum;
  char oemId[6];
  u8 revision;
  u32 rsdt;
} __attribute__((packed)) RSDP;

typedef struct {
  char signature[4];
  u32 length;
  u8 revision;
  u8 checksum;
  char oemId[6];
  char oemTableId[8];
  u32 oemRevision;
  u32 creatorId;
  u32 creatorRevision;
} __attribute__((packed)) SDTHeader;

void* acpiFindTable(char* signature);
void acpiInit();
void acpiReboot();
void acpiShutdown();

#endif