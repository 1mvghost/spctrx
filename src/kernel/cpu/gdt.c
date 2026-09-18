#include <gdt.h>

typedef struct {
  u16 limitLow;
  u16 baseLow;
  u8 baseMid;
  u8 access;
  u8 limitHigh : 4;
  u8 flags : 4;
  u8 baseHigh;
} __attribute__((packed)) GDTEntry;

typedef struct {
  u16 limit;
  GDTEntry* base;
} __attribute__((packed)) GDTR;

__attribute__((aligned(0x08))) static GDTEntry gdt[6];
static GDTR gdtr;

extern void gdtLoad(GDTR* gdtr);
extern void segReload();

void gdtSetDesc(u8 i, u32 limit, u32 base, u8 access, u8 flags) {
  GDTEntry* entry = &gdt[i];
  entry->baseLow = base & 0xffff;
  entry->baseMid = (base >> 16) & 0xff;
  entry->baseHigh = (base >> 24) & 0xff;
  entry->access = access;
  entry->limitLow = limit & 0xffff;
  entry->limitHigh = (limit >> 16) & 0xf;
  entry->flags = flags;
}

void gdtInit() {
  memset(gdt, 0, sizeof(gdt));

  gdtr.base = gdt;
  gdtr.limit = sizeof(gdt) - 1;

  gdtSetDesc(0, 0, 0, 0, 0);
  gdtSetDesc(1, 0x00ffffff, 0, 0x9a, 0xa);
  gdtSetDesc(2, 0x00ffffff, 0, 0x92, 0xc);
  gdtSetDesc(3, 0x00ffffff, 0, 0xfa, 0xa);
  gdtSetDesc(4, 0x00ffffff, 0, 0xf2, 0xc);

  gdtLoad(&gdtr);
  segReload(); /* change code to 0x08 and data to 0x10 */
}

void gdtMCpuInit() {
  gdtLoad(&gdtr);
  segReload(); /* change code to 0x08 and data to 0x10 */
}