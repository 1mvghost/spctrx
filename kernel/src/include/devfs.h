#ifndef DEVFS_H
#define DEVFS_H

#include <vfs.h>

int devfsWrite(FsNode* n, u64 pos, void* buf, size_t size);
void devfsMount(FsMnt* mnt);

#endif