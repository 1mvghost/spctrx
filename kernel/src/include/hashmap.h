#ifndef HASHMAP_H
#define HASHMAP_H

#include <ll.h>
#include <spinlock.h>

#define TABLE_SIZE 32

typedef struct {
  LLHead table[TABLE_SIZE];

  Spinlock lock;
} HashMap;

typedef struct {
  char key[256 + 1];
  void* value;

  LLHead head;
} HashMapNode;

void hashmapInit(HashMap* map);
void hashmapPut(HashMap* map, char* key, void* value);
void* hashmapGet(HashMap* map, char* key);

#endif