#include <printf.h>
#include <slab.h>
#include <string.h>
#include <tmpfs.h>
#include <vmm.h>

static SlabCache tmpfsNodeCache;
static FsHandler tmpfsHandler = {.create = tmpfsCreate,
                                 .lookup = tmpfsLookup,
                                 .mkdir = tmpfsMkdir,
                                 .readdir = tmpfsReaddir};

FsNode* tmpfsAllocNode(FsMnt* mnt, char* name, int type) {
  FsNode* node = vfsAllocNode(mnt);

  node->ops = &tmpfsHandler;
  node->type = type;

  TmpfsNode* tmpfsNode = slabAlloc(&tmpfsNodeCache);

  strcpy(tmpfsNode->name, name);

  if (type == TYPE_FILE) {
    tmpfsNode->data = vmmAlloc(1);
  }

  tmpfsNode->node = node;
  llInitHead(&tmpfsNode->children);
  llInitHead(&tmpfsNode->childHead);

  node->fsData = tmpfsNode;

  printf("tmpfs: new node for %s name %s type %d\n", mnt->path, name, type);

  return node;
}

int tmpfsAddChild(FsNode* n, FsNode* child) {
  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;
  TmpfsNode* tmpfsChild = (TmpfsNode*)child->fsData;

  llInsertFront(&tmpfsNode->children, &tmpfsChild->childHead);

  return 0;
}

bool tmpfsReaddir(FsNode* n, u64 pos, struct linux_dirent64* buf) {
  TmpfsNode* tmpfsNode = (TmpfsNode*)n->fsData;

  LLHead* current;
  LL_TRAVERSE(current, &tmpfsNode->children) {
    TmpfsNode* child = LIST_ENTRY(current, TmpfsNode, childHead);

    if (pos == 0) {
      strcpy(buf->d_name, child->name);
      buf->d_type = child->node->type;
      buf->d_reclen = sizeof(struct linux_dirent64) + strlen(buf->d_name) + 1;

      return true;
    }

    pos--;
  }
  return false;
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

  /**
   * todo: remove
   */
  tmpfsMkdir(mnt->root, "dev");

  printf("tmpfs: new mnt %s\n", mnt->path);
}

void tmpfsInit() {
  slabInitCache(&tmpfsNodeCache, "tmpfs nodes", sizeof(TmpfsNode));
}