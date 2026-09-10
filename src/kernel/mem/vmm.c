#include <pmm.h>
#include <printf.h>
#include <vmm.h>

static PageTable p4;

extern void vmmLoad(void* p4);

void* vmmPhysToVirt(u64 phys) {
  return (void*)(phys + 0xffff800000000000);
}

u64 vmmVirtToPhys(void* virt) {
  return ((u64)virt - 0xffff800000000000);
}

void vmmInvalidatePage(void* virt) {
  asm volatile("invlpg (%0)" ::"r"(virt) : "memory");
}

PageTable vmmGetPageTable(u64 ent) {
  if (ent == 0) {
    return 0;
  }
  return (PageTable)vmmPhysToVirt(ent & PAGE_ADDR_MASK);
}

PageTable vmmWalk(void* virt, bool alloc, int allocFlags) {
  int p4Idx = P4(virt);
  int p3Idx = P3(virt);
  int p2Idx = P2(virt);

  if (alloc && p4[p4Idx] == 0) {
    p4[p4Idx] = PTE(pmmAlloc(1), allocFlags);
  }
  PageTable p3 = vmmGetPageTable(p4[p4Idx]);

  if (alloc && p3[p3Idx] == 0) {
    p3[p3Idx] = PTE(pmmAlloc(1), allocFlags);
  }
  PageTable p2 = vmmGetPageTable(p3[p3Idx]);

  if (alloc && p2[p2Idx] == 0) {
    p2[p2Idx] = PTE(pmmAlloc(1), allocFlags);
  }
  PageTable p1 = vmmGetPageTable(p2[p2Idx]);

  return p1;
}

void vmmMap(void* virt, u64 phys, int flags) {
  PageTable p1 = vmmWalk(virt, true, flags);

  p1[P1(virt)] = PTE(phys, flags);

  vmmInvalidatePage(virt);
}

void vmmMapPages(void* virt, u64 phys, int flags, int n) {
  while (n--) {
    vmmMap(virt, phys, flags);
    virt += PAGE_SIZE, phys += PAGE_SIZE;
  }
}

void vmmUnmap(void* virt) {
  PageTable p1 = vmmWalk(virt, false, 0);
  if (p1 == 0)
    return;

  p1[P1(virt)] = 0;

  vmmInvalidatePage(virt);
}

void vmmUnmapPages(void* virt, int n) {
  while (n--) {
    vmmUnmap(virt);
    virt += PAGE_SIZE;
  }
}

void* vmmAlloc(u64 pages) {
  return vmmPhysToVirt(pmmAlloc(pages));
}

void vmmInit() {
  /* copy page tables from limine, there is no need to make new ones because
   * limine does everything beautifully */

  u64 p4Phys = 0;
  asm("movq %%cr3, %0" : "=r"(p4Phys));
  p4 = (PageTable)vmmPhysToVirt(p4Phys);

  vmmLoad((void*)p4Phys);
}