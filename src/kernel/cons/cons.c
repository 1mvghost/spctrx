#include <debug.h>
#include <fb.h>
#include <font.h>
#include <mem.h>

static u64 curX = 0;
static u64 curY = 0;

static u64 textWidth = 0;
static u64 textHeight = 0;

void consScroll() {
  u64 rows = textHeight / FHEIGHT;
  u64 rowPx = fbResX() * FHEIGHT;

  memcpy((void*)fbGetAddr(), (void*)fbGetAddr() + rowPx, rowPx * (rows - 1));

  /* clear last row */
  memset((u8*)(fbGetAddr() + (rows - 1) * rowPx), 0, rowPx);

  curY -= FHEIGHT;
  curX = 0;
}

void consNewline() {
  curY += FHEIGHT;
  curX = 0;

  if (curY >= textHeight) {
    consScroll();
  }
}

static SPINLOCK(putcSpinlock);
void consPutc(char ch) {
  mSpinlockAcquire(&putcSpinlock);

  switch (ch) {
    case '\n':
      consNewline();
      break;
    default:
      if (curX >= textWidth) {
        consNewline();
      }
      fontCh(curX, curY, ch);
      curX += FWIDTH;
      break;
  }
  debugPutc(ch);

  mSpinlockDrop(&putcSpinlock);
}
void consInit() {
  textWidth = ALIGN_DOWN(fbResX() / 4, FWIDTH);
  textHeight = ALIGN_DOWN(fbResY(), FHEIGHT);
}

/* printf hook */
void _putchar(char ch) {
  consPutc(ch);
}