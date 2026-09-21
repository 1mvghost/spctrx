#include <assert.h>
#include <debug.h>
#include <printf.h>
#include <slab.h>
#include <vmm.h>

/*
 * todo: make a pmm aligned alloc function so aligning down in slabFree() wont
 * bug with multiple pages
 */
#define SLAB_SIZE_PAGES 1

void slabAllocSlab(SlabCache* cache) {
  ASSERT(cache != 0);

  void* slab = vmmAlloc(SLAB_SIZE_PAGES);

  /*
   * initialize the slab's header
   */
  Slab* header = (Slab*)slab;

  header->start = header + 1;
  header->firstFree = header->start;
  llInitHead(&header->head);
  header->usedObj = 0;

  for (size_t i = 0; i < cache->objPerSlab - 1; i++) {
    *(u64*)(header->start + (i * cache->objSize)) =
        (u64)header->start + ((i + 1) * cache->objSize);
  }

  /*
   * insert it to the cache's linked list
   */
  llInsertFront(&cache->empty, &header->head);

  debug("slab: new slab for %s at %llx start:%llx firstfree:%llx\n",
        cache->name, slab, header->start, header->firstFree);
}

void* slabAlloc(SlabCache* cache) {
  ASSERT(cache != 0);

  mSpinlockAcquire(&cache->lock);

  LLHead* selected;

  if (llEmpty(&cache->partial) && llEmpty(&cache->empty)) {
    slabAllocSlab(cache);

    mSpinlockDrop(&cache->lock);

    return slabAlloc(cache);
  }

  if (llEmpty(&cache->empty)) {
    selected = &cache->partial;
  } else {
    selected = &cache->empty;
  }

  Slab* slab = LIST_ENTRY(selected->next, Slab, head);

  void* result = slab->firstFree;

  slab->firstFree = (void*)(*(u64*)slab->firstFree);
  slab->usedObj++;

  /*
   * move the slab to the full list of slabs
   */
  if (slab->usedObj == cache->objPerSlab) {
    llDelete(&slab->head);
    llInsertFront(&cache->full, &slab->head);
  }

  mSpinlockDrop(&cache->lock);

  return result;
}

void slabFree(SlabCache* cache, void* addr) {
  ASSERT(cache != 0);
  ASSERT(addr != 0);

  mSpinlockAcquire(&cache->lock);

  Slab* slab = (Slab*)ALIGN_DOWN((u64)addr, SLAB_SIZE_PAGES * PAGE_SIZE);

  *((u64*)addr) = (u64)slab->firstFree;

  slab->firstFree = addr;
  slab->usedObj--;

  llDelete(&slab->head);

  /*
   * switch slab states
   */
  if (slab->usedObj == 0) {
    llInsertFront(&cache->empty, &slab->head);
  } else {
    llInsertFront(&cache->partial, &slab->head);
  }

  mSpinlockDrop(&cache->lock);
}

void slabInitCache(SlabCache* cache, char* name, size_t objSize) {
  ASSERT(cache != 0);
  ASSERT(objSize > 0);

  objSize = ALIGN_UP(objSize, 8);

  cache->name = name;

  llInitHead(&cache->full);
  llInitHead(&cache->partial);
  llInitHead(&cache->empty);

  cache->objSize = objSize;
  cache->objPerSlab = ((SLAB_SIZE_PAGES * PAGE_SIZE) - sizeof(Slab)) / objSize;
  cache->lock = (Splock)ATOMIC_FLAG_INIT;

  debug("slab: new cache %s (%llx) objsize:%lld objperslab:%lld\n", name, cache,
        objSize, cache->objPerSlab);
}