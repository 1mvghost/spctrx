#include <cpu.h>
#include <gdt.h>
#include <idt.h>
#include <msr.h>
#include <panic.h>

void disableInts() {
  asm volatile("cli");
}

void enableInts() {
  asm volatile("sti");
}

void waitForInterrupt() {
  asm volatile("hlt");
}

void halt() {
  disableInts();

  for (;;) {
    waitForInterrupt();
  }
}

void cpuid(u32* eax, u32* ebx, u32* ecx, u32* edx) {
  asm volatile("cpuid" : "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(*eax));
}

void cpuEarlyInit() {
  gdtInit();
  idtInit();

  /**
   * check for msr support
   */
  if (!msrCheckSupport()) {
    panic("msr not supported!\n");
  }
}

void cpuApEarlyInit() {
  gdtFlush();
  idtFlush();
}