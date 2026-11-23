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

typedef struct {
  u8 entryType;
  u8 entryLength;
} __attribute__((packed)) MADTEntry;

typedef struct {
  SDTHeader header;

  u32 lapic;
  u32 flags;

  MADTEntry entries[1];
} __attribute__((packed)) MADT;

typedef struct {
  SDTHeader h;
  u32 sdtPtr[256];
} __attribute__((packed)) RSDT;

typedef struct {
  u8 addrSpace;
  u8 bitWidth;
  u8 bitOffset;
  u8 accessSize;
  u64 addr;
} __attribute__((packed)) GenericAddress;

typedef struct {
  SDTHeader h;
  u32 FirmwareCtrl;
  u32 Dsdt;
  // field used in ACPI 1.0; no longer in use, for compatibility only
  u8 Reserved;
  u8 PreferredPowerManagementProfile;
  u16 SCI_Interrupt;
  u32 SMI_CommandPort;
  u8 AcpiEnable;
  u8 AcpiDisable;
  u8 S4BIOS_REQ;
  u8 PSTATE_Control;
  u32 PM1aEventBlock;
  u32 PM1bEventBlock;
  u32 PM1aControlBlock;
  u32 PM1bControlBlock;
  u32 PM2ControlBlock;
  u32 PMTimerBlock;
  u32 GPE0Block;
  u32 GPE1Block;
  u8 PM1EventLength;
  u8 PM1ControlLength;
  u8 PM2ControlLength;
  u8 PMTimerLength;
  u8 GPE0Length;
  u8 GPE1Length;
  u8 GPE1Base;
  u8 CStateControl;
  u16 WorstC2Latency;
  u16 WorstC3Latency;
  u16 FlushSize;
  u16 FlushStride;
  u8 DutyOffset;
  u8 DutyWidth;
  u8 DayAlarm;
  u8 MonthAlarm;
  u8 Century;
  // reserved in ACPI 1.0; used since ACPI 2.0+
  u16 BootArchitectureFlags;
  u8 Reserved2;
  u32 Flags;
  // 12 byte structure; see below for details
  GenericAddress ResetReg;
  u8 ResetValue;
  u8 Reserved3[3];

  // 64bit pointers - Available on ACPI 2.0+
  u64 X_FirmwareControl;
  u64 X_Dsdt;
  GenericAddress X_PM1aEventBlock;
  GenericAddress X_PM1bEventBlock;
  GenericAddress X_PM1aControlBlock;
  GenericAddress X_PM1bControlBlock;
  GenericAddress X_PM2ControlBlock;
  GenericAddress X_PMTimerBlock;
  GenericAddress X_GPE0Block;
  GenericAddress X_GPE1Block;
} __attribute__((packed)) FADT;

void* acpiFindTable(char* signature);
void acpiInit();
void acpiReboot();
void acpiShutdown();

#endif