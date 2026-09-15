#ifndef VFS_H
#define VFS_H

#include <ll.h>
#include <util.h>

#define TYPE_DIR 1
#define TYPE_FILE 2

struct FsNode {
  u8 Type;
  struct FsMnt* Mnt;
  void* FsData;
  struct FsHandler* Ops;
};

struct FsMnt {
  char Type[32];
  char Dev[64];
  char Path[64];

  /* make lookups less painful */
  struct FsNode* Mountpoint;

  struct FsNode* Root;

  LLHead Head;
};

struct FsFd {
  struct FsNode* Inode;
  struct FsMnt* Mnt;
  u64 Pos;
  u64 Flags;
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
  int (*Open)(struct FsNode* n, u64 flags);
  int (*Read)(struct FsFd* fd, u8* buf, u64 size);
  int (*Write)(struct FsFd* fd, u8* buf, u64 size);
  void (*Close)(struct FsFd* fd);
  bool (*MkDir)(struct FsNode* n, char* name);
  struct FsNode* (*Lookup)(struct FsNode* n, char* name);
  bool (*ReadDir)(struct FsFd* fd, struct linux_dirent64* buf, u64 size);
};
void vfsInit();
struct FsNode* vfsAlloc(struct FsMnt* mnt, u8 type);
struct FsNode* vfsLookup(char* path);
struct FsFd* vfsFdAlloc(struct FsNode* n, u64 flags);
#endif