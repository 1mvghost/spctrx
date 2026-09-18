#ifndef SLAB_H
#define SLAB_H

#include <ll.h>

typedef struct {
  void* start;
  void* firstFree;

  LLHead head;

  size_t usedObj;
} Slab;

typedef struct {
  char* name;

  LLHead full;
  LLHead partial;
  LLHead empty;

  size_t objSize;
  size_t objPerSlab;
} SlabCache;

void* slabAlloc(SlabCache* cache);
void slabFree(SlabCache* cache, void* addr);
void slabInitCache(SlabCache* cache, char* name, size_t objSize);

#endif