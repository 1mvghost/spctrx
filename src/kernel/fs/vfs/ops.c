#include <ops.h>

struct FsFd* vfsOpen(char* path, u64 flags) {
  struct FsNode* n = vfsLookup(path);
  if (!n)
    return 0;
  if (n->ops && n->ops->open) {
    if (n->ops->open(n, flags)) {
      struct FsFd* fd = vfsFdAlloc(n, flags);
      if (!fd)
        return 0;

      return fd;
    } else {
      return 0;
    }

  } else {
    return 0;
  }
}

int vfsWrite(struct FsFd* fd, u8* buf, u64 size) {
  struct FsNode* n = fd->inode;
  if (n->ops && n->ops->write) {
    return n->ops->write(fd, buf, size);
  }
  return 0;
}

int vfsRead(struct FsFd* fd, u8* buf, u64 size) {
  struct FsNode* n = fd->inode;

  if (n->ops && n->ops->read) {
    return n->ops->read(fd, buf, size);
  }
  return 0;
}
bool vfsReadDir(struct FsFd* fd, struct linux_dirent64* buf, u64 size) {
  struct FsNode* n = fd->inode;
  if (n->ops && n->ops->readdir) {
    return n->ops->readdir(fd, buf, size);
  }
  return 0;
}