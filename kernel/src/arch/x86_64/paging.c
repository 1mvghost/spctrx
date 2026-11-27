#include <arch/x86_64/paging.h>
#include <paging.h>
#include <pmm.h>
#include <spinlock.h>
#include <string.h>
#include <vmm.h>

static PageTable p4;

extern void pagingLoad(void* p4);
extern void pagingInvalidatePage(void* virt);

int pagingTranslateFlags(int flags) {
  int result = PTE_PRESENT;

  if (flags & PAGE_WRITABLE) {
    result |= PTE_WRITABLE;
  }
  if (flags & PAGE_MMIO) {
    result |= PTE_NOT_CACHEABLE;
  }

  return result;
}

PageTable pagingGetPageTable(PageEntry ent) {
  if (ent == 0) {
    return 0;
  }
  return (PageTable)TO_HHDM(ent & PAGE_ADDR_MASK);
}

static SPINLOCK(walkSpinlock);
PageTable pagingWalk(void* virt, bool allocate, int allocFlags) {
  spinlockAcquire(&walkSpinlock);

  int p4Idx = P4(virt);
  int p3Idx = P3(virt);
  int p2Idx = P2(virt);

  if (allocate && p4[p4Idx] == 0) {
    u64 pt = pmmAlloc(1);
    memset(TO_HHDM(pt), 0, PAGE_SIZE);

    p4[p4Idx] = PTE(pt, allocFlags);
  }
  PageTable p3 = pagingGetPageTable(p4[p4Idx]);

  if (allocate && p3[p3Idx] == 0) {
    u64 pt = pmmAlloc(1);
    memset(TO_HHDM(pt), 0, PAGE_SIZE);

    p3[p3Idx] = PTE(pt, allocFlags);
  }
  PageTable p2 = pagingGetPageTable(p3[p3Idx]);

  if (allocate && p2[p2Idx] == 0) {
    u64 pt = pmmAlloc(1);
    memset(TO_HHDM(pt), 0, PAGE_SIZE);

    p2[p2Idx] = PTE(pt, allocFlags);
  }
  PageTable p1 = pagingGetPageTable(p2[p2Idx]);

  spinlockDrop(&walkSpinlock);
  return p1;
}

void archMapPage(void* virt, u64 phys, int flags) {
  int translatedFlags = pagingTranslateFlags(flags);

  PageTable p1 = pagingWalk(virt, true, translatedFlags);

  p1[P1(virt)] = PTE(phys, translatedFlags);

  pagingInvalidatePage(virt);
}

void archUnmapPage(void* virt) {
  PageTable p1 = pagingWalk(virt, false, 0);
  if (p1 == 0)
    return;

  p1[P1(virt)] = 0;

  pagingInvalidatePage(virt);
}

void pagingInit() {
  u64 p4Phys = 0;
  asm volatile("movq %%cr3, %0" : "=r"(p4Phys));

  p4 = (PageTable)TO_HHDM(p4Phys);

  pagingLoad((void*)p4Phys);
}