#ifndef VFS_H
#define VFS_H

#include <ll.h>
#include <util.h>

#define TYPE_DIR 1
#define TYPE_FILE 2

struct FsNode {
  u8 type;
  struct FsMnt* mnt;
  void* fsData;
  struct FsHandler* ops;
};

struct FsMnt {
  char type[32];
  char dev[64];
  char path[64];

  /* make lookups less painful */
  struct FsNode* mountpoint;

  struct FsNode* root;

  LLHead head;
};

struct FsFd {
  struct FsNode* inode;
  struct FsMnt* mnt;
  u64 pos;
  u64 flags;
};

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

struct FsHandler {
  int (*open)(struct FsNode* n, u64 flags);
  int (*read)(struct FsFd* fd, u8* buf, u64 size);
  int (*write)(struct FsFd* fd, u8* buf, u64 size);
  void (*close)(struct FsFd* fd);
  bool (*mkdir)(struct FsNode* n, char* name);
  struct FsNode* (*lookup)(struct FsNode* n, char* name);
  bool (*readdir)(struct FsFd* fd, struct linux_dirent64* buf, u64 size);
};
void vfsInit();
struct FsNode* vfsAlloc(struct FsMnt* mnt, u8 type);
struct FsNode* vfsLookup(char* path);
struct FsFd* vfsFdAlloc(struct FsNode* n, u64 flags);
#endif