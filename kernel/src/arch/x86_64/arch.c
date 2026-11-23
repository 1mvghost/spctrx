#include <arch.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <msr.h>
#include <panic.h>
#include <serial.h>

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

  /**
   * check for msr support
   */
  if (!msrCheckSupport()) {
    panic("msr not supported!\n");
  }
}

void archApEarlyInit() {
  gdtFlush();
  idtFlush();
}