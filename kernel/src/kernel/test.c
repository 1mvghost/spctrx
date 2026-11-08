#include <acpi.h>
#include <ahci.h>
#include <fb.h>
#include <gdt.h>
#include <idt.h>
#include <ioapic.h>
#include <irq.h>
#include <isr.h>
#include <lapic.h>
#include <limine.h>
#include <ll.h>
#include <mp.h>
#include <pci.h>
#include <pmm.h>
#include <printf.h>
#include <slab.h>
#include <spinlock.h>
#include <string.h>
#include <util.h>
#include <vfsops.h>
#include <vmm.h>

void tick() {
  printf("timer tick!\n");
  lapicSendEOI();
}

void test() {
#ifdef AHCI_TEST
  char buf[512];
  memset(buf, 0, sizeof(buf));

  printf("ahci: test\n");
  ahciRead(0, 0, 1, buf);
  for (int i = 0; i < 512; i++)
    printf("%c", buf[i]);
  printf("\n");
#endif
#ifdef SERIAL_TEST
  // debugPuts("serial test test\n");
  printf("serial printf test %d %d %d %x %x", 67, 69, 420, 0xdeadbeef,
         0xcafebabe);
#endif
#ifdef PMM_TEST
  u64* a = pmmAlloc(1);
  u64* b = pmmAlloc(1);
  printf("a:%x b:%x\n", a, b);

  pmmFree(a, 1);

  pmmAlloc(2);
  pmmAlloc(3553323);

#endif
#ifdef PRINT_TEST
  printf("test\n");
  char buf[67];
  memset(buf, 0, 67);
  sprintf(buf, "Hello world %x %x hi 123 %s %c\ns\n", 0xdeadbeef, 0xcafebabe,
          "string", '!');
  for (int i = 0; i < 67; i++) {
    printf("%c", buf[i]);
  }
  printf("hi!\n");
#endif
#ifdef CONS_TEST
  for (int i = 0; i < 100000; i++) {
    printf("abb");
  }
#endif
#ifdef VMM_TEST
  vmmMap(0, pmmAlloc(1), PTE_WRITABLE);
  *((char*)0) = '!';
  printf("%c\n", *((char*)0));
  vmmUnmap(0);
  printf("unmapped\n");
  printf("%c\n", *((char*)0));
#endif
#define APIC_TEST
#ifdef APIC_TEST
  int vector = irqAllocVector();

  irqInstall(vector, tick);

  /**
   * pit timer
   */
  ioApicMapISA(0, vector);
#endif
#define VFS_TEST
#ifdef VFS_TEST
  vfsCreate("/", "test");

  char* buf = "hello world!";
  vfsWrite("/test", 1, buf, 13);

  u8 read[14];
  vfsRead("/test", 0, &read, 14);

  printf("* read from /test *\n");
  for (u64 i = 0; i < sizeof(read); i++) {
    printf("%c", read[i]);
  }
  printf("\n");

  char* test = "* hello world from write to /dev/tty *\n";
  vfsWrite("/dev/tty", 0, test, strlen(test));
#endif
#ifdef SLAB_TEST
  SlabCache testCache;
  slabInitCache(&testCache, "test cache", 32, 0);

  for (int i = 0; i < 102; i++) {
    void* allocated = slabAlloc(&testCache);

    printf("%llx\n", allocated);

    if (i % 5 == 0) {
      slabFree(&testCache, allocated);
    }
  }
#endif
}