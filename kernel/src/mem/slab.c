#include <assert.h>
#include <printf.h>
#include <slab.h>
#include <string.h>
#include <vmm.h>

/**
 * the objects in a slab will be in the following format:
 * - actual object data
 * - pointer to the next free object, can be obtained easily with the
 * OBJ_NEXT_FREE macro
 */

/**
 * some helpful macros to make calculations easier
 */
#define SLAB_GET_OBJ(start, i, objSize) \
  (void*)((u64)start + (objSize + sizeof(u64)) * (i))
#define OBJ_NEXT_FREE(obj, objSize) *(u64*)((u64)obj + (objSize))

void slabAllocSlab(SlabCache* cache) {
  ASSERT(cache != 0);

  /**
   * allocate the slab
   */
  Slab* header = vmmAlloc(1);
  memset(header, 0, PAGE_SIZE);

  /*
   * initialize the slab's header
   */
  header->start = (char*)(header + 1);
  header->firstFree = header->start;
  llInitHead(&header->head);
  header->usedObj = 0;

  /**
   * initialize the slab's freelist
   */
  for (size_t i = 0; i < cache->objPerSlab - 1; i++) {
    void* current = SLAB_GET_OBJ(header->start, i, cache->objSize);

    if (cache->initializer != 0) {
      cache->initializer(current, cache->objSize);
    }

    OBJ_NEXT_FREE(current, cache->objSize) =
        (u64)SLAB_GET_OBJ(header->start, i + 1, cache->objSize);
  }

  /*
   * insert the slab to the cache's linked list
   */
  llInsertFront(&cache->empty, &header->head);

  printf("slab: new slab for %s at %llx start:%llx firstfree:%llx\n",
         cache->name, header, header->start, header->firstFree);
}

void* slabAlloc(SlabCache* cache) {
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

  void* result = slab->firstFree;

  slab->firstFree = (char*)OBJ_NEXT_FREE(slab->firstFree, cache->objSize);
  slab->usedObj++;

  /*
   * move the slab to the full list of slabs
   */
  if (slab->usedObj == cache->objPerSlab) {
    llDelete(&slab->head);
    llInsertFront(&cache->full, &slab->head);
  }

  spinlockDrop(&cache->lock);

  // printf("slab: allocated object addr %llx cache %s\n", result, cache->name);
  return result;
}

void slabFree(SlabCache* cache, void* addr) {
  ASSERT(cache != 0);
  ASSERT(addr != 0);

  spinlockAcquire(&cache->lock);

  Slab* slab = (Slab*)ALIGN_DOWN((u64)addr, PAGE_SIZE);

  OBJ_NEXT_FREE(addr, cache->objSize) = (u64)slab->firstFree;

  slab->firstFree = addr;
  slab->usedObj--;

  /*
   * switch slab states
   */
  llDelete(&slab->head);

  if (slab->usedObj == 0) {
    llInsertFront(&cache->empty, &slab->head);
  } else {
    llInsertFront(&cache->partial, &slab->head);
  }

  spinlockDrop(&cache->lock);
}

void slabInitCache(SlabCache* cache,
                   char* name,
                   size_t objSize,
                   SlabInitializer initializer) {
  ASSERT(cache != 0);
  ASSERT(objSize > 0);

  objSize = ALIGN_UP(objSize, 8);

  strncpy(cache->name, name, sizeof(cache->name) - 1);

  llInitHead(&cache->full);
  llInitHead(&cache->partial);
  llInitHead(&cache->empty);

  cache->objSize = objSize;
  cache->initializer = initializer;

  cache->objPerSlab = (PAGE_SIZE - sizeof(Slab)) / (objSize + sizeof(u64));
  ASSERT(cache->objPerSlab >= 1);

  spinlockInit(&cache->lock);

  printf("slab: new cache %s (%llx) objsize:%lld objperslab:%lld\n", name,
         cache, objSize, cache->objPerSlab);
}