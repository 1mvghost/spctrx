#include <assert.h>
#include <log.h>
#include <paging.h>
#include <pmm.h>
#include <slab.h>
#include <string.h>
#include <tmpfs.h>
#include <vmm.h>

#define DEBUG(fmt, ...) dprintf("tmpfs", fmt, __VA_ARGS__)

/**
 * TODO: get rid of this limit by implementing a simple page cache
 */
#define FILE_SIZE_PAGES 2

static SlabCache tmpfsNodeCache;
static FsHandler tmpfsHandler = {.read = tmpfsRead,
                                 .write = tmpfsWrite,
                                 .create = tmpfsCreate,
                                 .lookup = tmpfsLookup,
                                 .mkdir = tmpfsMkdir,
                                 .readdir = tmpfsReaddir};

FsNode* tmpfsAllocNode(FsMnt* mnt, int type) {
  FsNode* node = vfsAllocNode(mnt);

  node->ops = &tmpfsHandler;
  node->type = type;

  TmpfsNode* tmpfsNode = slabAlloc(&tmpfsNodeCache);

  if (type == TYPE_FILE) {
    tmpfsNode->data = TO_HHDM(pmmAlloc(FILE_SIZE_PAGES));
    memset(tmpfsNode->data, 0, FILE_SIZE_PAGES * PAGE_SIZE);
  }

  hashmapInit(&tmpfsNode->children);

  node->fsData = tmpfsNode;

  // DEBUG("new node for %s type %d\n", mnt->path, type);

  return node;
}

int tmpfsAddChild(FsNode* n, FsNode* child, char* name) {
  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;

  hashmapPut(&tmpfsNode->children, name, child);

  return 0;
}

int tmpfsReadWrite(FsNode* n, u64 pos, void* buf, size_t size, bool write) {
  if (n->type == TYPE_DIR) {
    return -1;
  }

  if ((pos + size) >= (FILE_SIZE_PAGES * PAGE_SIZE)) {
    return -1;
  }

  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;

  if (write) {
    memcpy(tmpfsNode->data + pos, buf, size);
  } else {
    memcpy(buf, tmpfsNode->data + pos, size);
  }

  return 0;
}

int tmpfsRead(FsNode* n, u64 pos, void* buf, size_t size) {
  return tmpfsReadWrite(n, pos, buf, size, false);
}

int tmpfsWrite(FsNode* n, u64 pos, void* buf, size_t size) {
  return tmpfsReadWrite(n, pos, buf, size, true);
}

int tmpfsReaddir(FsNode* n, u64 pos, struct linux_dirent64* buf) {
  ASSERT(buf != 0);

  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;

  char* name = hashmapGetItem(&tmpfsNode->children, pos);
  if (name == 0) {
    return -1;
  }

  FsNode* child = hashmapGet(&tmpfsNode->children, name);

  buf->d_ino = 0;
  strcpy(buf->d_name, name);
  buf->d_off = 0;
  buf->d_reclen = sizeof(struct linux_dirent64) + strlen(name) + 1;
  buf->d_type = child->type;

  return 0;
}

int tmpfsCreate(FsNode* n, char* name) {
  return tmpfsAddChild(n, tmpfsAllocNode(n->mnt, TYPE_FILE), name);
}

int tmpfsMkdir(FsNode* n, char* name) {
  return tmpfsAddChild(n, tmpfsAllocNode(n->mnt, TYPE_DIR), name);
}

FsNode* tmpfsLookup(FsNode* n, char* name) {
  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;

  return hashmapGet(&tmpfsNode->children, name);
}

void tmpfsMount(FsMnt* mnt) {
  mnt->root = tmpfsAllocNode(mnt, TYPE_DIR);

  DEBUG("new mnt %s\n", mnt->path);
}

void tmpfsInit() {
  slabInitCache(&tmpfsNodeCache, "tmpfs nodes", sizeof(TmpfsNode), 0);
}