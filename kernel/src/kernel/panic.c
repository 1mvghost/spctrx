#include <arch.h>
#include <log.h>
#include <panic.h>
#include <printf.h>
#include <stdarg.h>
#include <string.h>

#define DEBUG(fmt, ...) dprintf("panic", fmt, __VA_ARGS__)

void doPanic(char* err) {
  if (*err != '\0') {
    DEBUG("%s", err);
  }

  DEBUG("--- Kernel Call Trace ---\n", "");

  archStacktrace();

  DEBUG("call trace end, everything halted!\n", "");

  archHcf();
}

void panic(char* fmt, ...) {
  archDisableInterrupts();

  va_list va;
  va_start(va, fmt);

  char buf[1024];
  memset(buf, 0, sizeof(buf));
  vsnprintf(buf, sizeof(buf), fmt, va);

  doPanic(buf);

  va_end(va);
}