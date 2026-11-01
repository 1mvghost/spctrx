#ifndef LAPIC_H
#define LAPIC_H

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100  // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define PIC1_DATA 0x21
#define PIC2_DATA 0xa1

#define LAPIC_REGID 0x20
#define LAPIC_REGVER 0x30
#define LAPIC_REGEOI 0xb0
#define LAPIC_REGSPI 0xf0

void lapicSendEOI();
void lapicInit();

#endif