#include <idt.h>

typedef struct {
  u16 offsetLow;
  u16 selector;
  u8 ist;
  u8 attributes;
  u16 offsetMid;
  u32 offsetHigh;
  u32 reserved;
} __attribute__((packed)) IDTEntry;

typedef struct {
  u16 limit;
  IDTEntry* base;
} __attribute__((packed)) IDTR;

__attribute__((aligned(0x10))) static IDTEntry idt[256];
static IDTR idtr;

extern void idtLoad(IDTR* idtr);

void idtSetDesc(u8 i, void* isr, u8 flags) {
  IDTEntry* entry = &idt[i];
  entry->offsetLow = (u64)isr & 0xFFFF;
  entry->selector = 0x08;
  entry->attributes = flags;
  entry->offsetMid = ((u64)isr >> 16) & 0xFFFF;
  entry->offsetHigh = ((u64)isr >> 32) & 0xFFFFFFFF;
  entry->reserved = 0;
  entry->ist = 0;
}
void idtInit() {
  idtr.base = idt;
  idtr.limit = sizeof(idt) - 1;

  idtLoad(&idtr);
}
void idtMCpuInit() {
  idtLoad(&idtr);
}