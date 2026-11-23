#ifndef VMM_H
#define VMM_H

#include <boot.h>
#include <util.h>

/**
 * page flags
 */
#define PAGE_WRITABLE 1
#define PAGE_MMIO 2

#define TO_HHDM(phys) (void*)((phys) + getHHDM())
#define FROM_HHDM(virt) ((u64)(virt) - getHHDM())

void vmmMap(void* virt, u64 phys, int flags, size_t length);

#endif