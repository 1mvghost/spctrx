#include <boot.h>
#include <cpu.h>
#include <lapic.h>
#include <log.h>
#include <printf.h>

#define DEBUG(fmt, ...) dprintf("mp", fmt, __VA_ARGS__)

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_mp_request
    mpRequest = {.id = LIMINE_MP_REQUEST_ID, .revision = 4};

void mpEntry(struct limine_mp_info* mp) {
  UNUSED(mp);

  disableInts();

  cpuApEarlyInit();

  lapicInit();

  halt();
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