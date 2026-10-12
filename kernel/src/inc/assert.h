#ifndef ASSERT_H
#define ASSERT_H

#include <panic.h>

#define ASSERT(condition) \
  if (!(condition))       \
  panic("assert failed: %s:%d\n", __FILE__, __LINE__)

#endif