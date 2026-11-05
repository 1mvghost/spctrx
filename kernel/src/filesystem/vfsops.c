#include <vfs.h>
#include <vfsops.h>

int vfsWrite(char* path, u64 pos, void* buf, size_t size) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->write) {
    return node->ops->write(node, pos, buf, size);
  }
  return 0;
}

int vfsRead(char* path, u64 pos, void* buf, size_t size) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->read) {
    return node->ops->read(node, pos, buf, size);
  }
  return 0;
}

int vfsReadDir(char* path, u64 pos, struct linux_dirent64* buf) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->readdir) {
    return node->ops->readdir(node, pos, buf);
  }
  return 0;
}

int vfsCreate(char* path, char* name) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->create) {
    return node->ops->create(node, name);
  }
  return 0;
}

int vfsMkdir(char* path, char* name) {
  FsNode* node = vfsLookup(path);
  if (node && node->ops->mkdir) {
    return node->ops->mkdir(node, name);
  }
  return 0;
}