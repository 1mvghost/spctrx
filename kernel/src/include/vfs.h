#ifndef VFS_H
#define VFS_H

#include <ll.h>
#include <util.h>

#define TYPE_DIR 1
#define TYPE_FILE 2

typedef struct {
  int type;
  struct _FsMnt* mnt;
  void* fsData;
  struct _FsHandler* ops;

  LLHead head;
} FsNode;

typedef struct _FsMnt {
  char type[32 + 1];
  char dev[64 + 1];
  char path[64 + 1];

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
  int (*read)(FsNode* n, u64 pos, void* buf, size_t size);
  int (*write)(FsNode* n, u64 pos, void* buf, size_t size);
  int (*create)(FsNode* n, char* name);
  int (*mkdir)(FsNode* n, char* name);
  int (*readdir)(FsNode* n, u64 pos, struct linux_dirent64* buf);

  FsNode* (*lookup)(FsNode* n, char* name);
} FsHandler;

FsNode* vfsAllocNode(FsMnt* mnt);

FsNode* vfsLookup(char* path);

bool vfsMount(char* path, char* dev, char* type);

void vfsInit();

#endif