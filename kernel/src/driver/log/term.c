#include <fb.h>
#include <font.h>
#include <log.h>
#include <term.h>
#include <util.h>

#include <flanterm.h>
#include <flanterm_backends/fb.h>

#define DEBUG(fmt, ...) dprintf("term", fmt, __VA_ARGS__)

static struct flanterm_context* flantermCtx;

static u32 fg = 0x00ffffff;

static bool termInitialized = false;

void termInit() {
  struct limine_framebuffer* fb = fbGetMainFb();

  flantermCtx = flanterm_fb_init(
      NULL, NULL, fb->address, fb->width, fb->height, fb->pitch,
      fb->red_mask_size, fb->red_mask_shift, fb->green_mask_size,
      fb->green_mask_shift, fb->blue_mask_size, fb->blue_mask_shift, NULL, NULL,
      NULL, NULL, &fg, NULL, NULL, &font, FONT_WIDTH, FONT_HEIGHT, 0, 0, 0, 0,
      0, true);

  if (flantermCtx == 0) {
    DEBUG("failed to initialize flanterm!\n", "");
    return;
  }

  termInitialized = true;
}

void termPutc(char ch) {
  if (!termInitialized) {
    return;
  }

  /**
   * newlines are broken without \r
   */
  if (ch == '\n') {
    flanterm_write(flantermCtx, "\r", 1);
  }

  flanterm_write(flantermCtx, &ch, 1);
}