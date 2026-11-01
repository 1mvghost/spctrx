#ifndef TMPFS_H
#define TMPFS_H

#include <util.h>
#include <vfs.h>

typedef struct {
  char name[32 + 1];
  FsNode* node;

  LLHead childHead;
  LLHead children;

  void* data;
} TmpfsNode;

bool tmpfsReaddir(FsNode* n, u64 pos, struct linux_dirent64* buf);
int tmpfsCreate(FsNode* n, char* name);
int tmpfsMkdir(FsNode* n, char* name);
FsNode* tmpfsLookup(FsNode* n, char* name);

void tmpfsMount(FsMnt* mnt);
void tmpfsInit();

#endif