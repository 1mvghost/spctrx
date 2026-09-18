#include <debug.h>
#include <dev.h>
#include <vfs.h>

struct FsNode* dirdebug;

struct FsNode* devLookup(struct FsNode* n, char* name) {
  UNUSED(n);
  if (!strcmp(name, "dbg")) {
    return dirdebug;
  }
  return 0;
}

bool devReadDir(struct FsFd* fd, struct linux_dirent64* buf, u64 size) {
  if (!fd)
    return 0;
  if (fd->Mnt->Root != fd->Inode) {
    return 0;
  }
  if (fd->Pos > 0) {
    return 0;
  }
  if (size != 1) {
    return 0;
  }
  strcpy(buf->d_name, "dbg");
  buf->d_reclen = sizeof(struct linux_dirent64) + strlen("dbg");
  buf->d_type = fd->Inode->Type;
  buf->d_ino = 67;
  return 1;
}
int devOpen(struct FsNode* n, u64 flags) {
  UNUSED(n);
  UNUSED(flags);
  return 1;
}
int devWrite(struct FsFd* fd, u8* buf, u64 size) {
  if (fd->Inode == dirdebug) {
    for (u64 i = 0; i < size; i++) {
      debugPutc(buf[i]);
    }
    return size;
  }
  return 0;
}

struct FsHandler devHandler = {.Lookup = devLookup,
                               .Open = devOpen,
                               .Write = devWrite,
                               .ReadDir = devReadDir};
void devInit(struct FsMnt* mnt) {
  debug("devfs: mnt is %s\n", mnt->Path);

  mnt->Root = vfsAlloc(mnt, TYPE_DIR);
  mnt->Root->Ops = &devHandler;

  dirdebug = vfsAlloc(mnt, TYPE_FILE);
  dirdebug->Ops = &devHandler;
}