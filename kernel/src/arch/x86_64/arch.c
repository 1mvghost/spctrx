#include <arch.h>
#include <arch/x86_64/apic.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/isr.h>
#include <arch/x86_64/msr.h>
#include <arch/x86_64/paging.h>
#include <arch/x86_64/serial.h>
#include <panic.h>

void archDisableInterrupts() {
  asm volatile("cli");
}

void archEnableInterrupts() {
  asm volatile("sti");
}

void archWaitForInterrupt() {
  asm volatile("hlt");
}

void archHcf() {
  archDisableInterrupts();

  for (;;) {
    archWaitForInterrupt();
  }
}

void archEarlyInit() {
  gdtInit();
  idtInit();
  isrInit();

  serialInit();

  pagingInit();

  /**
   * check for msr support
   */
  if (!msrCheckSupport()) {
    panic("msr not supported!\n");
  }
}

void archInit() {
  apicInit();
}

void archApEarlyInit() {
  gdtFlush();
  idtFlush();
}