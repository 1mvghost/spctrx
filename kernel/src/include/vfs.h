#ifndef VFS_H
#define VFS_H

#include <ll.h>
#include <util.h>

#define TYPE_DIR 1
#define TYPE_FILE 2

typedef struct {
  u8 type;
  struct _FsMnt* mnt;
  void* fsData;
  struct _FsHandler* ops;

  LLHead head;
} FsNode;

typedef struct _FsMnt {
  char type[32];
  char dev[64];
  char path[64];

  /* make lookups less painful */
  FsNode* mountpoint;

  FsNode* root;

  LLHead head;
  LLHead nodes;
} FsMnt;

/* not used for much for now, will be more useful when i start making userspace
 */
/* taken from
 * https://github.com/torvalds/linux/blob/master/include/linux/dirent.h */
struct linux_dirent64 {
  u64 d_ino;
  int d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[];
};

typedef struct _FsHandler {
  int (*open)(FsNode* n, u64 flags);
  int (*read)(FsNode* n, u64 pos, u8* buf, u64 size);
  int (*write)(FsNode* n, u64 pos, u8* buf, u64 size);
  int (*create)(FsNode* n, FsNode* child);
  int (*mkdir)(FsNode* n, char* name);

  bool (*readdir)(FsNode* n, u64 pos, struct linux_dirent64* buf, u64 size);

  FsNode* (*lookup)(FsNode* n, char* name);
} FsHandler;

FsNode* vfsAllocNode(FsMnt* mnt);

FsNode* vfsLookup(char* path);

bool vfsMount(char* path, char* dev, char* type);

void vfsInit();

#endif