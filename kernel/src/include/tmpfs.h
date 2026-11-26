#ifndef TMPFS_H
#define TMPFS_H

#include <hashmap.h>
#include <util.h>
#include <vfs.h>

typedef struct {
  HashMap children;

  u8* data;
} TmpfsNode;

FsNode* tmpfsAllocNode(FsMnt* mnt, int type);

int tmpfsRead(FsNode* n, u64 pos, void* buf, size_t size);
int tmpfsWrite(FsNode* n, u64 pos, void* buf, size_t size);
int tmpfsReaddir(FsNode* n, u64 pos, struct linux_dirent64* buf);
int tmpfsCreate(FsNode* n, char* name);
int tmpfsMkdir(FsNode* n, char* name);
FsNode* tmpfsLookup(FsNode* n, char* name);

void tmpfsMount(FsMnt* mnt);
void tmpfsInit();

#endif