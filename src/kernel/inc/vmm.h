#ifndef VMM_H
#define VMM_H

#include <paging.h>
#include <util.h>

void vmmInit();

void vmmMapPages(void* virt, u64 phys, int flags, int n);
void vmmUnmapPages(void* virt, int n);

void vmmMap(void* virt, u64 phys, int flags);
void vmmUnmap(void* virt);

u64 vmmVirtToPhys(void* virt);
void* vmmPhysToVirt(u64 phys);

void* vmmAlloc(u64 pages);

#endif