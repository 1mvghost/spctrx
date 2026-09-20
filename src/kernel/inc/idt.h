#ifndef IDT_H
#define IDT_H
#include <util.h>

void idtSetDesc(u8 i, void* isr, u8 flags);
void idtInit();
void idtFlush();

#endif