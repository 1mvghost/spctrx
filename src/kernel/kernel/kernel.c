#include <acpi.h>
#include <assert.h>
#include <boot.h>
#include <cons.h>
#include <debug.h>
#include <fb.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <limine.h>
#include <mp.h>
#include <panic.h>
#include <pci.h>
#include <pmm.h>
#include <printf.h>
#include <test.h>
#include <util.h>
#include <vfs.h>
#include <vmm.h>

__attribute__((
    used,
    section(".limine_requests"))) static volatile u64 limine_base_revision[] =
    LIMINE_BASE_REVISION(4);

void main() {
  disableInts();

  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == 0) {
    halt();
  }

  debugInit();

  gdtInit();
  idtInit();
  isrInit();

  fbInit();
  consInit();

  printf("spctrx v0.1 (www.github.com/1mvghost/spctrx)\n");
  printf("64-Bit Long Mode\n\n");

  pmmInit();
  vmmInit();

  vfsInit();
  pciInit();
  acpiInit();
  mpInit();

  test();

  keypress();
  acpiReboot();

  panic("\n");
}