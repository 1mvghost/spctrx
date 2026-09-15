#include <alloc.h>
#include <debug.h>
#include <dev.h>
#include <ll.h>
#include <mem.h>
#include <ops.h>
#include <printf.h>
#include <tmp.h>
#include <vfs.h>
#include <vmm.h>

static LLHead mnts;
static struct FsNode* root;

struct FsNode* vfsAlloc(struct FsMnt* mnt, u8 type) {
  struct FsNode* n = malloc(sizeof(struct FsNode));
  memset(n, 0, sizeof(struct FsNode));

  n->Type = type;
  n->Mnt = mnt;
  n->Ops = 0;

  if (mnt->Root)
    n->Ops = mnt->Root->Ops;

  return n;
}
struct FsMnt* vfsFindMnt(struct FsNode* n) {
  if (!n)
    return 0;

  LLHead* curr;
  LL_TRAVERSE(curr, &mnts) {
    struct FsMnt* mnt = LIST_ENTRY(curr, struct FsMnt, Head);
    if (mnt->Mountpoint == n) {
      return mnt;
    }
  }

  return 0;
}
struct FsNode* vfsLook(struct FsNode* cur, char* name) {
  if (!cur)
    return 0;
  if (cur->Type == TYPE_FILE)
    return 0;
  if (!(cur->Ops && cur->Ops->Lookup))
    return 0;

  struct FsNode* c = cur->Ops->Lookup(cur, name);

  /* mnt check */
  struct FsMnt* mnt = vfsFindMnt(c);
  if (mnt)
    c = mnt->Root;

  return c;
}
struct FsNode* vfsLookup(char* path) {
  if (!path)
    return 0;

  struct FsNode* cur = root;

  struct FsMnt* mnt = vfsFindMnt(cur);
  if (mnt)
    cur = mnt->Root;

  char sp[64];
  sp[0] = '\0';

  char* p = path;
  int i = 0;

  while (*p) {
    if (*p == '/') {
      if (i > 0) {
        cur = vfsLook(cur, sp);

        if (!cur)
          return 0;
        if (cur->Type == TYPE_FILE)
          return 0;
      }

      sp[0] = '\0';
      i = 0;

    } else {
      if (i >= 63)
        continue;
      sp[i] = *p;
      i++;
      sp[i] = '\0';
    }

    ++p;
  }

  /* check if there's anything left to handle */
  if (sp[0] != '\0')
    cur = vfsLook(cur, sp);

  return cur;
}

struct FsFd* vfsFdAlloc(struct FsNode* n, u64 flags) {
  struct FsFd* fd = malloc(sizeof(struct FsFd));
  fd->Inode = n;
  fd->Pos = 0;
  fd->Mnt = n->Mnt;
  fd->Flags = flags;
  return fd;
}

Splock mntSplock = ATOMIC_FLAG_INIT;

void vfsMount(char* path, char* dev, char* type) {
  struct FsNode* l = vfsLookup(path);

  if (!l) {
    debug("vfs: cannot mnt on nonexistent path!!\n");
    return;
  }

  mSpinlockAcquire(&mntSplock);

  struct FsMnt* mnt = malloc(sizeof(struct FsMnt));
  memset(mnt, 0, sizeof(struct FsMnt));

  strcpy(mnt->Type, type);
  strcpy(mnt->Dev, dev);
  strcpy(mnt->Path, path);

  mnt->Mountpoint = l;

  llInitHead(&mnt->Head);

  if (!strcmp(type, "dev")) {
    devInit(mnt);
  }
  if (!strcmp(type, "tmp")) {
    tmpInit(mnt);
  }

  llInsertFront(&mnts, &mnt->Head);

  debug("vfs: MOUNTED %s\n", path);

  mSpinlockDrop(&mntSplock);
}

void vfsInit() {
  llInitHead(&mnts);

  root = malloc(sizeof(struct FsNode));

  vfsMount("/", "", "tmp");
  vfsMount("/dev", "", "dev");
}