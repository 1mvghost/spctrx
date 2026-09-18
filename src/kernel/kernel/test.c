#include <acpi.h>
#include <ahci.h>
#include <debug.h>
#include <fb.h>
#include <font.h>
#include <gdt.h>
#include <ide.h>
#include <idt.h>
#include <isr.h>
#include <limine.h>
#include <ll.h>
#include <mem.h>
#include <mp.h>
#include <ops.h>
#include <pci.h>
#include <pmm.h>
#include <printf.h>
#include <util.h>
#include <vmm.h>

void test() {
// #define IDE_TEST
#ifdef IDE_TEST
  printf("ide: test\n");
  u8* buf = calloc(512);
  ideRead(0, 0, 1, buf);
  for (int i = 0; i < 512; i++)
    printf("%c", buf[i]);
  free(buf);
  printf("\n");
#endif
// #define AHCI_TEST
#ifdef AHCI_TEST
  u8* buf = calloc(512);
  printf("ahci: test\n");
  buf = calloc(512);
  ahciRead(0, 0, 1, buf);
  for (int i = 0; i < 512; i++)
    printf("%c", buf[i]);
  free(buf);
  printf("\n");
#endif
#ifdef SERIAL_TEST
  // debugPuts("serial test test\n");
  debug("serial printf test %d %d %d %x %x", 67, 69, 420, 0xdeadbeef,
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
#define VFS_TEST
#ifdef VFS_TEST
  struct FsFd* fd = vfsOpen("/dev/dbg", 0x67);
  if (!fd) {
    return;
  }

  char* buf = "SERIAL TEST\n";
  printf("vfs: wrote %d bytes\n", vfsWrite(fd, (u8*)buf, strlen(buf)));

  printf("* readdir on /dev *\n");
  struct FsFd* fd2 = vfsOpen("/dev", 0x03);
  char testt[1024];
  struct linux_dirent64* dent = (struct linux_dirent64*)testt;

  vfsReadDir(fd2, dent, 1);
  printf("%s\n", dent->d_name);

  printf("* readdir on / *\n");
  struct FsFd* fd3 = vfsOpen("/", 0x03);
  char testtt[1024];
  struct linux_dirent64* dentt = (struct linux_dirent64*)testtt;

  vfsReadDir(fd3, dentt, 1);

  printf("%s\n", dentt->d_name);

#endif
#ifdef CONS_TEST
  for (int i = 0; i < 100000; i++) {
    printf("abb");
  }
#endif
#ifdef LL_TEST
  LinkedList ll;
  llInit(&ll);

  llAdd(&ll, "Hello World test");
  llAdd(&ll, "Linked list test 123");
  llAdd(&ll, "Linked list test 123");
  llAdd(&ll, "Linked list test 123");
  llAdd(&ll, "Linked list test 123");
  llAdd(&ll, "Linked list test 123");
  llAdd(&ll, "Linked list test 123");

  LLNode* cur = ll.Head;
  while (cur) {
    printf("%s\n", cur->Data);
    cur = cur->Next;
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
}