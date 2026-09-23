#include <assert.h>
#include <bitmap.h>
#include <debug.h>

#define INDEX(pos) (pos / 64)
#define BIT(pos) (pos % 64)

void bitmapInit(Bitmap* bitmap, size_t bits, u64* data) {
  ASSERT(bitmap != 0);
  ASSERT(bits > 0);
  ASSERT(data != 0);

  bitmap->bits = bits;
  bitmap->dataSize = bitmapCalculateSize(bits);
  bitmap->data = data;

  memset(bitmap->data, 0, bitmap->dataSize);

  debug("bitmap: new bitmap at %llx bits:%lld data:%llx\n", bitmap, bits, data);
}

void bitmapSet(Bitmap* bitmap, size_t pos, bool value) {
  ASSERT(bitmap != 0);
  ASSERT(pos < bitmap->bits);

  if (value) {
    bitmap->data[INDEX(pos)] |= (1 << BIT(pos));
  } else {
    bitmap->data[INDEX(pos)] &= ~(1 << BIT(pos));
  }
}

bool bitmapCheck(Bitmap* bitmap, size_t pos) {
  ASSERT(bitmap != 0);
  ASSERT(pos < bitmap->bits);

  return (bitmap->data[INDEX(pos)] & (1 << BIT(pos))) != 0;
}

void bitmapFillAll(Bitmap* bitmap, bool value) {
  ASSERT(bitmap != 0);

  if (value) {
    memset(bitmap->data, 0xf, bitmap->dataSize);
  } else {
    memset(bitmap->data, 0, bitmap->dataSize);
  }
}

void bitmapFill(Bitmap* bitmap, size_t from, size_t to, bool value) {
  ASSERT(bitmap != 0);
  ASSERT(from < bitmap->bits);
  ASSERT(to <= bitmap->bits);

  for (size_t pos = from; pos < to; pos++) {
    bitmapSet(bitmap, pos, value);
  }
}

size_t bitmapFind(Bitmap* bitmap, size_t length) {
  ASSERT(bitmap != 0);
  ASSERT(length > 0);

  size_t start = 0;
  size_t current = 0;

  for (size_t pos = 1; pos < bitmap->bits; pos++) {
    if (!bitmapCheck(bitmap, pos)) {
      if (start == 0) {
        start = pos;
      }
      current++;
    } else {
      start = 0;
      current = 0;
    }

    if (current == length) {
      break;
    }
  }
  return start;
}

size_t bitmapCalculateSize(size_t bits) {
  return (ALIGN_UP(bits, 64) / sizeof(u64));
}