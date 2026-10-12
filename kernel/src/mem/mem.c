#include <mem.h>

void mSpinlockAcquire(Spinlock* lock) {
  while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
    asm("pause");
  }
}

void mSpinlockDrop(Spinlock* lock) {
  atomic_flag_clear_explicit(lock, memory_order_release);
}

void mSpinlockInit(Spinlock* lock) {
  *lock = (Spinlock)ATOMIC_FLAG_INIT;
}