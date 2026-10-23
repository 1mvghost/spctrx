#include <assert.h>
#include <printf.h>
#include <slab.h>
#include <string.h>
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

  header->start = (char*)(header + 1);
  header->firstFree = header->start;
  llInitHead(&header->head);
  header->usedObj = 0;

  for (size_t i = 0; i < cache->objPerSlab - 1; i++) {
    *(size_t*)(header->start + (i * cache->objSize)) =
        (size_t)header->start + ((i + 1) * cache->objSize);
  }

  /*
   * insert it to the cache's linked list
   */
  llInsertFront(&cache->empty, &header->head);

  printf("slab: new slab for %s at %llx start:%llx firstfree:%llx\n",
         cache->name, slab, header->start, header->firstFree);
}

void* slabAlloc(SlabCache* cache) {
  /**
   * the objects in a slab will be in the following format:
   * - pointer to the next free object (size_t)
   * - actual object data
   *
   * this may not be ideal, but will work for now
   */
  ASSERT(cache != 0);

  spinlockAcquire(&cache->lock);

  LLHead* selected;

  if (llEmpty(&cache->partial) && llEmpty(&cache->empty)) {
    slabAllocSlab(cache);

    spinlockDrop(&cache->lock);

    return slabAlloc(cache);
  }

  if (llEmpty(&cache->empty)) {
    selected = &cache->partial;
  } else {
    selected = &cache->empty;
  }

  Slab* slab = LIST_ENTRY(selected->next, Slab, head);

  char* result = slab->firstFree;

  slab->firstFree = (char*)(*(size_t*)slab->firstFree);
  slab->usedObj++;

  /*
   * move the slab to the full list of slabs
   */
  if (slab->usedObj == cache->objPerSlab) {
    llDelete(&slab->head);
    llInsertFront(&cache->full, &slab->head);
  }

  spinlockDrop(&cache->lock);

  return result + sizeof(size_t);
}

void slabFree(SlabCache* cache, void* addr) {
  ASSERT(cache != 0);
  ASSERT(addr != 0);

  spinlockAcquire(&cache->lock);

  Slab* slab = (Slab*)ALIGN_DOWN((size_t)addr, SLAB_SIZE_PAGES * PAGE_SIZE);

  *((size_t*)addr) = (size_t)slab->firstFree;

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

  spinlockDrop(&cache->lock);
}

void slabInitCache(SlabCache* cache, char* name, size_t objSize) {
  ASSERT(cache != 0);
  ASSERT(objSize > 0);

  objSize = ALIGN_UP(objSize, 8) + sizeof(size_t);

  /**
   * keep the name within bounds
   */
  ASSERT((size_t)strlen(name) < sizeof(cache->name));

  strcpy(cache->name, name);

  llInitHead(&cache->full);
  llInitHead(&cache->partial);
  llInitHead(&cache->empty);

  cache->objSize = objSize;

  cache->objPerSlab = ((SLAB_SIZE_PAGES * PAGE_SIZE) - sizeof(Slab)) / objSize;
  ASSERT(cache->objPerSlab >= 1);

  spinlockInit(&cache->lock);

  printf("slab: new cache %s (%llx) objsize:%lld objperslab:%lld\n", name,
         cache, objSize, cache->objPerSlab);
}