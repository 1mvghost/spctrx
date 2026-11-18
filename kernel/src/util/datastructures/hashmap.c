#include <assert.h>
#include <hashmap.h>
#include <log.h>
#include <slab.h>
#include <string.h>
#include <util.h>

#define DEBUG(fmt, ...) dprintf("hashmap", fmt, __VA_ARGS__)

static SlabCache hashmapNodeCache;
static bool hashmapCacheInitialized = false;

void hashmapNodeInitializer(void* buf, size_t size) {
  UNUSED(size);

  HashMapNode* node = (HashMapNode*)buf;

  node->key[sizeof(node->key) - 1] = '\0';

  llInitHead(&node->head);
}

void hashmapInit(HashMap* map) {
  ASSERT(map != 0);

  if (!hashmapCacheInitialized) {
    slabInitCache(&hashmapNodeCache, "hashmap nodes", sizeof(HashMapNode),
                  hashmapNodeInitializer);
    hashmapCacheInitialized = true;
  }

  spinlockInit(&map->lock);

  /**
   * initialize all chains
   */
  for (int i = 0; i < TABLE_SIZE; i++) {
    llInitHead(&map->table[i]);
  }

  DEBUG("new hashmap at %llx\n", map);
}

u64 hashmapHash(char* key) {
  /**
   * TODO: replace with a better hashing algorithm
   */

  u64 result = 0;
  for (int i = 1; *key != '\0'; i++, key++) {
    result += i * (char)*key;
  }

  return result;
}

void hashmapPut(HashMap* map, char* key, void* value) {
  ASSERT(map != 0);
  ASSERT(key != 0);

  u64 hash = hashmapHash(key);

  spinlockAcquire(&map->lock);

  /**
   * check for an entry with the same key, replace the value if found
   * if didn't find anything, allocate an entry
   */
  LLHead* chain = &map->table[hash % TABLE_SIZE];
  LLHead* current;

  bool replaced = false;

  LL_TRAVERSE(current, chain) {
    HashMapNode* node = LIST_ENTRY(current, HashMapNode, head);

    if (!strcmp(node->key, key)) {
      node->value = value;

      replaced = true;
    }
  }

  if (!replaced) {
    HashMapNode* node = slabAlloc(&hashmapNodeCache);

    strncpy(node->key, key, sizeof(node->key) - 1);
    node->value = value;

    llInsertFront(chain, &node->head);
  }

  spinlockDrop(&map->lock);
}

void* hashmapGet(HashMap* map, char* key) {
  ASSERT(map != 0);
  ASSERT(key != 0);

  u64 hash = hashmapHash(key);

  LLHead* chain = &map->table[hash % TABLE_SIZE];
  LLHead* current;

  LL_TRAVERSE(current, chain) {
    HashMapNode* node = LIST_ENTRY(current, HashMapNode, head);

    if (!strcmp(node->key, key)) {
      return node->value;
    }
  }

  return 0;
}