#include <debug.h>
#include <dev.h>
#include <tmp.h>
#include <util.h>
#include <vfs.h>

struct FsNode* dirdev;

int tmpOpen(struct FsNode* n, u64 flags) {
  UNUSED(n);
  UNUSED(flags);
  return 1;
}
struct FsNode* tmpLookup(struct FsNode* n, char* name) {
  UNUSED(n);
  if (!strcmp(name, "dev")) {
    return dirdev;
  }
  return 0;
}

bool tmpReadDir(struct FsFd* fd, struct linux_dirent64* buf, u64 size) {
  if (!fd)
    return 0;
  if (fd->mnt->root != fd->inode) {
    return 0;
  }
  if (fd->pos > 0) {
    return 0;
  }
  if (size != 1) {
    return 0;
  }
  strcpy(buf->d_name, "dev");
  buf->d_reclen = sizeof(struct linux_dirent64) + strlen("dev");
  buf->d_type = fd->inode->type;
  buf->d_ino = 67;
  return 1;
}
struct FsHandler tmpHandler = {.lookup = tmpLookup,
                               .readdir = tmpReadDir,
                               .open = tmpOpen};
void tmpInit(struct FsMnt* mnt) {
  debug("tmpfs: mnt is %s\n", mnt->path);

  mnt->root = vfsAlloc(mnt, TYPE_DIR);
  mnt->root->ops = &tmpHandler;

  dirdev = vfsAlloc(mnt, TYPE_DIR);
  dirdev->ops = &tmpHandler;
}