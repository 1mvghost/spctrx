#include <ll.h>
#include <printf.h>
#include <slab.h>
#include <string.h>
#include <vfs.h>
#include <vmm.h>

static LLHead mnts;
static FsNode* root;

static SlabCache nodeCache;

FsMnt* vfsFindMnt(FsNode* n) {
  if (!n) {
    return 0;
  }

  LLHead* current;

  LL_TRAVERSE(current, &mnts) {
    FsMnt* mnt = LIST_ENTRY(current, FsMnt, head);
    if (mnt->mountpoint == n) {
      return mnt;
    }
  }

  return 0;
}

FsNode* vfsLook(FsNode* current, char* name) {
  if (!current) {
    return 0;
  }
  if (current->type == TYPE_FILE) {
    return 0;
  }
  if (!current->ops || !current->ops->lookup) {
    return 0;
  }

  FsNode* currNode = current->ops->lookup(current, name);

  /* mnt check */
  FsMnt* mnt = vfsFindMnt(currNode);
  if (mnt) {
    currNode = mnt->root;
  }

  return currNode;
}

FsNode* vfsLookup(char* path) {
  if (!path) {
    return 0;
  }

  FsNode* current = root;

  FsMnt* mnt = vfsFindMnt(current);
  if (mnt) {
    current = mnt->root;
  }

  char sp[64];
  sp[0] = '\0';

  char* p = path;
  int i = 0;

  while (*p) {
    if (*p == '/') {
      if (i > 0) {
        current = vfsLook(current, sp);

        if (!current || current->type == TYPE_FILE) {
          return 0;
        }
      }

      sp[0] = '\0';
      i = 0;

    } else {
      if (i >= 63) {
        continue;
      }
      sp[i] = *p;
      i++;
      sp[i] = '\0';
    }

    ++p;
  }

  /* check if there's anything left to handle */
  if (sp[0] != '\0') {
    current = vfsLook(current, sp);
  }

  return current;
}

static SlabCache mntCache;

bool vfsMount(char* path, char* dev, char* type) {
  FsNode* lookedUp = vfsLookup(path);

  if (!lookedUp) {
    printf("vfs: cannot mount on nonexistent path %s\n", path);
    return false;
  }

  FsMnt* mnt = slabAlloc(&mntCache);

  strcpy(mnt->type, type);
  strcpy(mnt->dev, dev);
  strcpy(mnt->path, path);

  mnt->mountpoint = lookedUp;
  mnt->root = 0;

  llInitHead(&mnt->head);

  llInsertFront(&mnts, &mnt->head);

  printf("vfs: mounted %s fs %s\n", path, type);

  return true;
}

void vfsDoMounts() {
  vfsMount("/", "", "tmp");
  vfsMount("/dev", "", "dev");
}

void vfsInit() {
  slabInitCache(&nodeCache, "vfs nodes", sizeof(FsNode));
  slabInitCache(&mntCache, "vfs mounts", sizeof(FsMnt));

  llInitHead(&mnts);

  root = slabAlloc(&nodeCache);

  vfsDoMounts();
}