#include <assert.h>
#include <boot.h>
#include <debug.h>
#include <panic.h>
#include <pmm.h>
#include <vmm.h>

static u64 here = 0;
static u64 left = 0;

void pmmHandleOOM() {
  panic("OUT OF MEMORY\n");
}

u64 pmmAlloc(size_t pages) {
  ASSERT(pages > 0);

  if (pages > left) {
    pmmHandleOOM();
    return 0;
  }

  u64 result = here;

  here += (PAGE_SIZE * pages);
  left -= pages;

  return result;
}

u64 pmmZeroAlloc(size_t pages) {
  ASSERT(pages > 0);

  u64 addr = pmmAlloc(pages);
  memset(vmmPhysToVirt(addr), 0, pages * PAGE_SIZE);

  return addr;
}

void pmmFree(u64 addr, size_t pages) {
  UNUSED(addr);
  UNUSED(pages);
}

void pmmInit() {
  int mMapLen = limineMMapRequest().response->entry_count;

  u64 mx = 0;
  struct limine_memmap_entry* selected;

  for (int i = 0; i < mMapLen; i++) {
    struct limine_memmap_entry* ent = limineMMapRequest().response->entries[i];
    if (ent->type == LIMINE_MEMMAP_USABLE) {
      if (ent->length > mx) {
        mx = ent->length;
        selected = ent;
      }
    }
  }

  here = selected->base;
  left = mx / PAGE_SIZE;

  debug("pmm: start:%x pages:%d\n", here, left);
}