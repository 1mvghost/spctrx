#ifndef TMPFS_H
#define TMPFS_H

#include <util.h>
#include <vfs.h>

typedef struct {
  char name[32];
  FsNode* node;

  LLHead childHead;
  LLHead children;

  void* data;
} TmpfsNode;

int tmpfsCreate(FsNode* n, char* name);
int tmpfsMkdir(FsNode* n, char* name);
FsNode* tmpfsLookup(FsNode* n, char* name);

void tmpfsMount(FsMnt* mnt);
void tmpfsInit();

#endif