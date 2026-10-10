#ifndef IRQ_H
#define IRQ_H

#include <util.h>

void irqHandle(int irq);
int irqAllocVector();
void irqInstall(int vector, void* handler);
void irqInit();

#endif