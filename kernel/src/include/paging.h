#ifndef PAGING_H
#define PAGING_H

#include <util.h>

#define PAGE_SIZE 0x1000  // 4096

void archMapPage(void* virt, u64 phys, int flags);
void archUnmapPage(void* virt);

#endif