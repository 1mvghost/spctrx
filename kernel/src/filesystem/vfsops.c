#include <vfs.h>
#include <vfsops.h>

int vfsWrite(char* path, u64 pos, void* buf, u64 size) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->write) {
    return node->ops->write(node, pos, buf, size);
  }
  return 0;
}

int vfsRead(char* path, u64 pos, void* buf, u64 size) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->read) {
    return node->ops->read(node, pos, buf, size);
  }
  return 0;
}

bool vfsReadDir(char* path, u64 pos, struct linux_dirent64* buf, u64 size) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->readdir) {
    return node->ops->readdir(node, pos, buf, size);
  }
  return false;
}