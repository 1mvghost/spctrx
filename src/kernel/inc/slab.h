#ifndef SLAB_H
#define SLAB_H

#include <ll.h>

typedef struct {
  void* Start;
  void* FirstFree;

  LLHead Head;

  size_t UsedObj;
} Slab;

typedef struct {
  char* Name;

  LLHead Full;
  LLHead Partial;
  LLHead Empty;

  size_t ObjSize;
  size_t ObjPerSlab;
} SlabCache;

void* slabAlloc(SlabCache* cache);
void slabFree(SlabCache* cache, void* addr);
void slabInitCache(SlabCache* cache, char* name, size_t objSize);

#endif