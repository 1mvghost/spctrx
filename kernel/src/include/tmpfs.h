#ifndef TMPFS_H
#define TMPFS_H

#include <util.h>
#include <vfs.h>

typedef struct {
  char name[32 + 1];
  FsNode* node;

  LLHead childHead;
  LLHead children;

  u8* data;
} TmpfsNode;

FsNode* tmpfsAllocNode(FsMnt* mnt, char* name, int type);

size_t tmpfsRead(FsNode* n, u64 pos, u8* buf, u64 size);
size_t tmpfsWrite(FsNode* n, u64 pos, u8* buf, u64 size);
bool tmpfsReaddir(FsNode* n, u64 pos, struct linux_dirent64* buf);
int tmpfsCreate(FsNode* n, char* name);
int tmpfsMkdir(FsNode* n, char* name);
FsNode* tmpfsLookup(FsNode* n, char* name);

void tmpfsMount(FsMnt* mnt);
void tmpfsInit();

#endif