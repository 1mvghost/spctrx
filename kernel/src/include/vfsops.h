#ifndef VFSOPS_H
#define VFSOPS_H

#include <util.h>
#include <vfs.h>

int vfsWrite(char* path, u64 pos, void* buf, size_t size);
int vfsRead(char* path, u64 pos, void* buf, size_t size);
int vfsReadDir(char* path, u64 pos, struct linux_dirent64* buf);
int vfsCreate(char* path, char* name);
int vfsMkdir(char* path, char* name);

#endif