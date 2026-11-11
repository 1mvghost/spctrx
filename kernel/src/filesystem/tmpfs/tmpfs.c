#include <log.h>
#include <slab.h>
#include <string.h>
#include <tmpfs.h>
#include <vmm.h>

#define DEBUG(fmt, ...) dprintf("tmpfs", fmt, __VA_ARGS__)

/**
 * todo: get rid of this
 */
#define FILE_SIZE_PAGES 2

static SlabCache tmpfsNodeCache;
static FsHandler tmpfsHandler = {.read = tmpfsRead,
                                 .write = tmpfsWrite,
                                 .create = tmpfsCreate,
                                 .lookup = tmpfsLookup,
                                 .mkdir = tmpfsMkdir,
                                 .readdir = tmpfsReaddir};

FsNode* tmpfsAllocNode(FsMnt* mnt, char* name, int type) {
  FsNode* node = vfsAllocNode(mnt);

  node->ops = &tmpfsHandler;
  node->type = type;

  TmpfsNode* tmpfsNode = slabAlloc(&tmpfsNodeCache);

  strncpy(tmpfsNode->name, name, sizeof(tmpfsNode->name) - 1);

  if (type == TYPE_FILE) {
    tmpfsNode->data = vmmAlloc(FILE_SIZE_PAGES);
    memset(tmpfsNode->data, 0, FILE_SIZE_PAGES * PAGE_SIZE);
  }

  tmpfsNode->node = node;
  llInitHead(&tmpfsNode->children);
  llInitHead(&tmpfsNode->childHead);

  node->fsData = tmpfsNode;

  DEBUG("new node for %s name %s type %d\n", mnt->path, name, type);

  return node;
}

int tmpfsAddChild(FsNode* n, FsNode* child) {
  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;
  TmpfsNode* tmpfsChild = (TmpfsNode*)child->fsData;

  llInsertFront(&tmpfsNode->children, &tmpfsChild->childHead);

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
  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;

  LLHead* current;
  LL_TRAVERSE(current, &tmpfsNode->children) {
    TmpfsNode* child = LIST_ENTRY(current, TmpfsNode, childHead);

    if (pos == 0) {
      strcpy(buf->d_name, child->name);
      buf->d_type = child->node->type;
      buf->d_reclen = sizeof(struct linux_dirent64) + strlen(buf->d_name) + 1;

      return 0;
    }

    pos--;
  }
  return -1;
}

int tmpfsCreate(FsNode* n, char* name) {
  return tmpfsAddChild(n, tmpfsAllocNode(n->mnt, name, TYPE_FILE));
}

int tmpfsMkdir(FsNode* n, char* name) {
  return tmpfsAddChild(n, tmpfsAllocNode(n->mnt, name, TYPE_DIR));
}

FsNode* tmpfsLookup(FsNode* n, char* name) {
  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;

  LLHead* current;
  LL_TRAVERSE(current, &tmpfsNode->children) {
    TmpfsNode* child = LIST_ENTRY(current, TmpfsNode, childHead);

    if (!strcmp(child->name, name)) {
      return child->node;
    }
  }
  return 0;
}

void tmpfsMount(FsMnt* mnt) {
  mnt->root = tmpfsAllocNode(mnt, "", TYPE_DIR);

  DEBUG("new mnt %s\n", mnt->path);
}

void tmpfsInit() {
  slabInitCache(&tmpfsNodeCache, "tmpfs nodes", sizeof(TmpfsNode), 0);
}