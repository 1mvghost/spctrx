#include <assert.h>
#include <string.h>

void memset(void* dst, char value, int n) {
  ASSERT(dst != 0);
  ASSERT(n > 0);

  asm volatile("rep stosb" : : "a"(value), "c"(n));
}

void* memcpy(void* dst, void* src, int n) {
  ASSERT(dst != 0);
  ASSERT(src != 0);
  ASSERT(n > 0);

  asm volatile("rep movsb" : : "c"(n));

  return dst;
}

void* memmove(void* dstptr, const void* srcptr, size_t size) {
  ASSERT(dstptr != 0);
  ASSERT(srcptr != 0);
  ASSERT(size > 0);

  u8* dst = (u8*)dstptr;
  const u8* src = (const u8*)srcptr;

  if (dst < src) {
    for (size_t i = 0; i < size; i++)
      dst[i] = src[i];
  } else {
    for (size_t i = size; i != 0; i--)
      dst[i - 1] = src[i - 1];
  }
  return dstptr;
}

int memcmp(void* a, void* b, size_t cnt) {
  /*
   * https://github.com/gcc-mirror/gcc/blob/master/libiberty/memcmp.c
   */
  ASSERT(a != 0);
  ASSERT(b != 0);
  ASSERT(cnt > 0);

  u8* ptrA = (u8*)a;
  u8* ptrB = (u8*)b;

  while (cnt--) {
    if (*ptrA++ != *ptrB++) {
      return ptrA[-1] > ptrB[-1] ? 1 : -1;
    }
  }
  return 0;
}

int strlen(char* s) {
  ASSERT(s != 0);

  char* ptr = s;
  int res = 0;

  while (*ptr) {
    res++;
    ptr++;
  }
  return res;
}

int strcmp(const char* s1, const char* s2) {
  ASSERT(s1 != 0);
  ASSERT(s2 != 0);

  const char* ss1 = s1;
  const char* ss2 = s2;
  while (*ss1 && (*ss1 == *ss2)) {
    ss1++;
    ss2++;
  }
  return *(const char*)ss1 - *(const char*)ss2;
}

char* strcpy(char* dst, char* src) {
  /*
   * https://linux.die.net/man/3/strcpy
   */

  ASSERT(dst != 0);
  ASSERT(src != 0);

  size_t i = 0;
  for (; src[i] != '\0'; i++) {
    dst[i] = src[i];
  }

  dst[i] = '\0';
  return dst;
}