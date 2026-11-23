#include <arch.h>
#include <arch/x86_64/lapic.h>
#include <boot.h>
#include <log.h>

#define DEBUG(fmt, ...) dprintf("mp", fmt, __VA_ARGS__)

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_mp_request
    mpRequest = {.id = LIMINE_MP_REQUEST_ID, .revision = 4};

void mpEntry(struct limine_mp_info* mp) {
  UNUSED(mp);

  archDisableInterrupts();

  archApEarlyInit();

  lapicInit();

  archHcf();
}

void mpInit() {
  if (mpRequest.response == 0) {
    DEBUG("no extra cpus!\n", "");
    return;
  }

  struct limine_mp_response* m = mpRequest.response;
  DEBUG("found %d cpus\n", m->cpu_count);

  for (u64 i = 0; i < m->cpu_count; i++) {
    m->cpus[i]->goto_address = mpEntry;
  }
}