#ifndef DEVFS_H
#define DEVFS_H

#include <vfs.h>

size_t devfsWrite(FsNode* n, u64 pos, u8* buf, u64 size);
void devfsMount(FsMnt* mnt);

#endif