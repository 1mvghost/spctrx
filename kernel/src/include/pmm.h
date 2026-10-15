#ifndef PMM_H
#define PMM_H

#include <util.h>

u64 pmmAlloc(size_t pages);
u64 pmmZeroAlloc(size_t pages);
void pmmFree(u64 addr, size_t pages);
void pmmInit();

#endif