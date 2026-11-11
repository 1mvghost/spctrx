#include <fb.h>
#include <log.h>
#include <panic.h>

#define DEBUG(fmt, ...) dprintf("fb", fmt, __VA_ARGS__)

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebufferRequest = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

static struct limine_framebuffer* fb;

struct limine_framebuffer* fbGetMainFb() {
  return fb;
}

void fbInit() {
  if (framebufferRequest.response == 0 ||
      framebufferRequest.response->framebuffers == 0) {
    panic("no framebuffers found!\n");
  }

  fb = framebufferRequest.response->framebuffers[0];

  DEBUG("PITCH:%d HEIGHT:%d ADDR:%x\n", fb->pitch, fb->height, fb->address);
}