#include <boot.h>
#include <debug.h>
#include <gdt.h>
#include <idt.h>
#include <stdatomic.h>

void mpEntry(struct limine_mp_info* mp) {
  UNUSED(mp);

  gdtFlush();
  idtFlush();

  asm("cli");
  asm("hlt");
}

void mpInit() {
  struct limine_mp_response* m = limineMpRequest().response;
  debug("mp: found %d cpus\n", m->cpu_count);

  for (u64 i = 0; i < m->cpu_count; i++) {
    atomic_store(&m->cpus[i]->goto_address, &mpEntry);
  }
}