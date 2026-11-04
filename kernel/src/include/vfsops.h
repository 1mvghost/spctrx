#ifndef VFSOPS_H
#define VFSOPS_H

#include <util.h>
#include <vfs.h>

size_t vfsWrite(char* path, u64 pos, void* buf, u64 size);
size_t vfsRead(char* path, u64 pos, void* buf, u64 size);
bool vfsReadDir(char* path, u64 pos, struct linux_dirent64* buf);
int vfsCreate(char* path, char* name);
int vfsMkdir(char* path, char* name);

#endif