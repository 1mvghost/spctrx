#include <serial.h>
#include <term.h>

/* printf hook */
void _putchar(char ch) {
  serialPutc(ch);
  termPutc(ch);
}