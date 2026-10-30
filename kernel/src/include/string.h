#ifndef STRING_H
#define STRING_H

#include <util.h>

void memset(void* dst, u8 value, size_t n);
void* memcpy(void* dst, void* src, size_t n);
void* memmove(void* dstptr, const void* srcptr, size_t size);
int memcmp(void* a, void* b, size_t cnt);
size_t strlen(char* s);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dst, char* src);

#endif