#ifndef BITMAP_H
#define BITMAP_H

#include <util.h>

typedef struct {
  size_t bits;
  size_t dataSize;

  u64* data;
} Bitmap;

void bitmapInit(Bitmap* bitmap, size_t bits, u64* data);

void bitmapSet(Bitmap* bitmap, size_t pos, bool value);

bool bitmapCheck(Bitmap* bitmap, size_t pos);

void bitmapFillAll(Bitmap* bitmap, bool value);

void bitmapFill(Bitmap* bitmap, size_t from, size_t to, bool value);

size_t bitmapFind(Bitmap* bitmap, size_t length);

size_t bitmapCalculateSize(size_t bits);

#endif