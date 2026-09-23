#include <assert.h>
#include <bitmap.h>
#include <boot.h>
#include <debug.h>
#include <panic.h>
#include <pmm.h>
#include <vmm.h>

static Bitmap pmmBitmap;

void pmmHandleOOM() {
  panic("OUT OF MEMORY\n");
}

u64 pmmAlloc(size_t pages) {
  ASSERT(pages > 0);

  u64 found = bitmapFind(&pmmBitmap, pages);

  if (!found) {
    pmmHandleOOM();
    return 0;
  }

  bitmapFill(&pmmBitmap, found, found + pages, true);

  debug("pmm: allocated page at %llx\n", found * PAGE_SIZE);
  return found * PAGE_SIZE;
}

u64 pmmZeroAlloc(size_t pages) {
  ASSERT(pages > 0);

  u64 addr = pmmAlloc(pages);
  memset(vmmPhysToVirt(addr), 0, pages * PAGE_SIZE);

  return addr;
}

void pmmFree(u64 addr, size_t pages) {
  ASSERT(addr != 0);
  ASSERT(pages > 0);

  size_t page = addr / PAGE_SIZE;
  bitmapFill(&pmmBitmap, page, page + pages, false);
}

void pmmInit() {
  int mMapLen = limineMMapRequest().response->entry_count;

  u64 last = 0;
  u64 bitmapAddr = 0;

  /*
   * get the last usable memory address
   */
  for (int i = 0; i < mMapLen; i++) {
    struct limine_memmap_entry* ent = limineMMapRequest().response->entries[i];
    if (ent->type == LIMINE_MEMMAP_USABLE) {
      last = ent->base + ent->length;
    }
  }

  size_t bitmapSize = bitmapCalculateSize(last / PAGE_SIZE);

  /*
   * find a memory range big enough for the bitmap's data
   */
  for (int i = 0; i < mMapLen; i++) {
    struct limine_memmap_entry* ent = limineMMapRequest().response->entries[i];
    if (ent->type == LIMINE_MEMMAP_USABLE && ent->base != 0 &&
        ent->length >= bitmapSize) {
      bitmapAddr = ent->base;
      break;
    }
  }

  if (!bitmapAddr) {
    panic("not enough memory for the pmm bitmap!\n");
  }

  debug("pmm: bitmap addr %llx\n", bitmapAddr);

  bitmapInit(&pmmBitmap, last / PAGE_SIZE, vmmPhysToVirt(bitmapAddr));
  bitmapFillAll(&pmmBitmap, true);

  /*
   * mark usable pages as free
   */
  for (int i = 0; i < mMapLen; i++) {
    struct limine_memmap_entry* ent = limineMMapRequest().response->entries[i];
    if (ent->base % PAGE_SIZE == 0 && ent->type == LIMINE_MEMMAP_USABLE) {
      pmmFree(ent->base, ent->length / PAGE_SIZE);
    }
  }

  /*
   * mark bitmap pages as reserved
   */
  size_t bitmapStartPage = bitmapAddr / PAGE_SIZE;
  size_t bitmapEndPage = bitmapStartPage + DIV_ROUND_UP(bitmapSize, PAGE_SIZE);

  bitmapFill(&pmmBitmap, bitmapStartPage, bitmapEndPage, true);
}