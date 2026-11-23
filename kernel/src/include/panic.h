#ifndef PANIC_H
#define PANIC_H

void archStacktrace();

void panic(char* fmt, ...);

#endif