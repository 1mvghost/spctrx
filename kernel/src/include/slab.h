#ifndef SLAB_H
#define SLAB_H

#include <ll.h>
#include <spinlock.h>

typedef void (*SlabInitializer)(void* buf, size_t size);

typedef struct {
  char* start;
  char* firstFree;

  LLHead head;

  size_t usedObj;
} Slab;

typedef struct {
  char name[64 + 1];

  LLHead full;
  LLHead partial;
  LLHead empty;

  size_t objSize;
  size_t objPerSlab;

  SlabInitializer initializer;

  Spinlock lock;
} SlabCache;

void* slabAlloc(SlabCache* cache);
void slabFree(SlabCache* cache, void* addr);
void slabInitCache(SlabCache* cache,
                   char* name,
                   size_t objSize,
                   SlabInitializer initializer);
#endif