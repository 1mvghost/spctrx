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
  if (fd->Mnt->Root != fd->Inode) {
    return 0;
  }
  if (fd->Pos > 0) {
    return 0;
  }
  if (size != 1) {
    return 0;
  }
  strcpy(buf->d_name, "dev");
  buf->d_reclen = sizeof(struct linux_dirent64) + strlen("dev");
  buf->d_type = fd->Inode->Type;
  buf->d_ino = 67;
  return 1;
}
struct FsHandler tmpHandler = {.Lookup = tmpLookup,
                               .ReadDir = tmpReadDir,
                               .Open = tmpOpen};
void tmpInit(struct FsMnt* mnt) {
  debug("tmpfs: mnt is %s\n", mnt->Path);

  mnt->Root = vfsAlloc(mnt, TYPE_DIR);
  mnt->Root->Ops = &tmpHandler;

  dirdev = vfsAlloc(mnt, TYPE_DIR);
  dirdev->Ops = &tmpHandler;
}