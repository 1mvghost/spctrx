#ifndef VFSOPS_H
#define VFSOPS_H

#include <util.h>

int vfsWrite(char* path, u64 pos, void* buf, u64 size);
int vfsRead(char* path, u64 pos, void* buf, u64 size);
bool vfsReadDir(char* path, u64 pos, struct linux_dirent64* buf, u64 size);

#endif