#include <acpi.h>
#include <arch.h>
#include <boot.h>
#include <fb.h>
#include <irq.h>
#include <limine.h>
#include <mp.h>
#include <pci.h>
#include <pmm.h>
#include <printf.h>
#include <term.h>
#include <util.h>
#include <vfs.h>

__attribute__((
    used,
    section(".limine_requests"))) static volatile u64 limine_base_revision[] =
    LIMINE_BASE_REVISION(4);

void main() {
  archDisableInterrupts();

  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == 0) {
    archHcf();
  }

  archEarlyInit();

  fbInit();
  termInit();

  printf("spctrx v0.1 (www.github.com/1mvghost/spctrx)\n");
  printf("64-Bit Long Mode\n\n");

  parseBootInfo();

  pmmInit();
  irqInit();
  vfsInit();
  acpiInit();
  archInit();
  pciInit();
  mpInit();

  vfsTreeIterate(vfsLookup("/"), 0);

  archEnableInterrupts();

  for (;;) {
    archWaitForInterrupt();
  }
}