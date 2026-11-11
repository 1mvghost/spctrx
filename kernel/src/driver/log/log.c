#include <printf.h>
#include <serial.h>
#include <stdarg.h>
#include <term.h>

void dprintf(char* prefix, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);

  char buf[1024];
  vsnprintf(buf, sizeof(buf), fmt, va);

  printf("%s: %s", prefix, buf);

  va_end(va);
}

/* printf hook */
void _putchar(char ch) {
  serialPutc(ch);
  termPutc(ch);
}