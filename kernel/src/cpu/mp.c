#include <apic.h>
#include <boot.h>
#include <debug.h>
#include <gdt.h>
#include <idt.h>
#include <mem.h>
#include <stdatomic.h>

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_mp_request
    mpRequest = {.id = LIMINE_MP_REQUEST_ID, .revision = 4};

static SPINLOCK(mpInitSpinlock);
void mpEntry(struct limine_mp_info* mp) {
  UNUSED(mp);

  disableInts();

  mSpinlockAcquire(&mpInitSpinlock);

  gdtFlush();
  idtFlush();

  apicApInit();

  mSpinlockDrop(&mpInitSpinlock);

  halt();
}

void mpInit() {
  if (mpRequest.response == 0) {
    debug("mp: no extra cpus!\n");
    return;
  }

  struct limine_mp_response* m = mpRequest.response;
  debug("mp: found %d cpus\n", m->cpu_count);

  for (u64 i = 0; i < m->cpu_count; i++) {
    m->cpus[i]->goto_address = mpEntry;
  }
}