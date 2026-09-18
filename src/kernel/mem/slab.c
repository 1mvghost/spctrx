#include <assert.h>
#include <debug.h>
#include <slab.h>
#include <vmm.h>

#define SLAB_SIZE_PAGES 4

void slabAllocSlab(SlabCache* cache) {
  ASSERT(cache != 0);

  void* slab = vmmAlloc(SLAB_SIZE_PAGES);

  /*
   * initialize the slab's header
   */
  Slab* header = (Slab*)slab;

  header->Start = header + 1;
  header->FirstFree = header->Start;
  llInitHead(&header->Head);
  header->UsedObj = 0;

  for (size_t i = 0; i < cache->ObjPerSlab - 1; i++) {
    *(u64*)(header->Start + (i * cache->ObjSize)) =
        (u64)header->Start + ((i + 1) * cache->ObjSize);
  }

  /*
   * insert it to the cache's linked list
   */
  llInsertFront(&cache->Empty, &header->Head);

  debug("slab: new slab for %s at %llx start:%llx firstfree:%llx\n",
        cache->Name, slab, header->Start, header->FirstFree);
}

void* slabAlloc(SlabCache* cache) {
  ASSERT(cache != 0);

  LLHead* selected;

  if (llEmpty(&cache->Partial) && llEmpty(&cache->Empty)) {
    slabAllocSlab(cache);
    return slabAlloc(cache);
  }

  if (llEmpty(&cache->Empty)) {
    selected = &cache->Partial;
  } else {
    selected = &cache->Empty;
  }

  Slab* slab = LIST_ENTRY(selected->Next, Slab, Head);

  void* result = slab->FirstFree;

  slab->FirstFree = (void*)(*(u64*)slab->FirstFree);
  slab->UsedObj++;

  /*
   * move the slab to the full list of slabs
   */
  if (slab->UsedObj == cache->ObjPerSlab) {
    llDelete(&slab->Head);
    llInsertFront(&cache->Full, &slab->Head);
  }

  return result;
}

void slabFree(SlabCache* cache, void* addr) {
  ASSERT(cache != 0);
  ASSERT(addr != 0);

  Slab* slab = (Slab*)ALIGN_DOWN((u64)addr, SLAB_SIZE_PAGES * PAGE_SIZE);

  *((u64*)addr) = (u64)slab->FirstFree;

  slab->FirstFree = addr;
  slab->UsedObj--;

  llDelete(&slab->Head);

  /*
   * switch slab states
   */
  if (slab->UsedObj == 0) {
    llInsertFront(&cache->Empty, &slab->Head);
  } else {
    llInsertFront(&cache->Partial, &slab->Head);
  }
}

void slabInitCache(SlabCache* cache, char* name, size_t objSize) {
  ASSERT(cache != 0);
  ASSERT(objSize > 0);

  objSize = ALIGN_UP(objSize, 8);

  cache->Name = name;

  llInitHead(&cache->Full);
  llInitHead(&cache->Partial);
  llInitHead(&cache->Empty);

  cache->ObjSize = objSize;
  cache->ObjPerSlab = ((SLAB_SIZE_PAGES * PAGE_SIZE) - sizeof(Slab)) / objSize;

  debug("slab: new cache %s (%llx) objsize:%lld objperslab:%lld\n", name, cache,
        objSize, cache->ObjPerSlab);
}