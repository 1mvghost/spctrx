#include <boot.h>
#include <debug.h>
#include <fb.h>
#include <limine.h>

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebufferRequest = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

static u64 x;
static u64 y;
static u64 addr;

u64 fbResX() {
  return x;
}
u64 fbResY() {
  return y;
}
u64 fbGetAddr() {
  return addr;
}

void fbInit() {
  if (framebufferRequest.response == 0 ||
      framebufferRequest.response->framebuffers == 0) {
    /**
     * todo: dont halt directly!
     */
    disableInts();
    halt();
  }
  struct limine_framebuffer* fb = framebufferRequest.response->framebuffers[0];

  addr = (u64)fb->address;
  x = (u64)fb->pitch;
  y = (u64)fb->height;

  debug("fb: PITCH:%d HEIGHT:%d ADDR:%x\n", x, y, addr);
}