#include <assert.h>
#include <bitmap.h>
#include <boot.h>
#include <debug.h>
#include <mem.h>
#include <panic.h>
#include <pmm.h>
#include <string.h>
#include <vmm.h>

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_memmap_request
    mMapRequest = {.id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0};

static Bitmap pmmBitmap;
static SPINLOCK(pmmSpinlock);

void pmmHandleOOM() {
  panic("OUT OF MEMORY\n");
}

u64 pmmAlloc(size_t pages) {
  ASSERT(pages > 0);

  mSpinlockAcquire(&pmmSpinlock);

  u64 found = bitmapFind(&pmmBitmap, pages);

  if (!found) {
    pmmHandleOOM();
    return 0;
  }

  bitmapFill(&pmmBitmap, found, found + pages, true);

  debug("pmm: allocated page at %llx\n", found * PAGE_SIZE);

  mSpinlockDrop(&pmmSpinlock);
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

  mSpinlockAcquire(&pmmSpinlock);

  size_t page = addr / PAGE_SIZE;
  bitmapFill(&pmmBitmap, page, page + pages, false);

  mSpinlockDrop(&pmmSpinlock);
}

void pmmInit() {
  if (mMapRequest.response == 0) {
    panic("memory map response unavailable!!\n");
  }

  int mMapLen = mMapRequest.response->entry_count;

  u64 last = 0;
  u64 bitmapAddr = 0;

  /*
   * get the last usable memory address
   */
  for (int i = 0; i < mMapLen; i++) {
    struct limine_memmap_entry* ent = mMapRequest.response->entries[i];
    if (ent->type == LIMINE_MEMMAP_USABLE) {
      last = ent->base + ent->length;
    }
  }

  size_t bitmapSize = bitmapCalculateSize(last / PAGE_SIZE);

  /*
   * find a memory range big enough for the bitmap's data
   */
  for (int i = 0; i < mMapLen; i++) {
    struct limine_memmap_entry* ent = mMapRequest.response->entries[i];
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
    struct limine_memmap_entry* ent = mMapRequest.response->entries[i];
    if (ent->base % PAGE_SIZE == 0 && ent->type == LIMINE_MEMMAP_USABLE) {
      bitmapFill(&pmmBitmap, ent->base / PAGE_SIZE,
                 (ent->base + ent->length) / PAGE_SIZE, false);
    }
  }

  /*
   * mark bitmap pages as reserved
   */
  size_t bitmapStartPage = bitmapAddr / PAGE_SIZE;
  size_t bitmapEndPage = bitmapStartPage + DIV_ROUND_UP(bitmapSize, PAGE_SIZE);

  bitmapFill(&pmmBitmap, bitmapStartPage, bitmapEndPage, true);

  bitmapSet(&pmmBitmap, 0, true);
}