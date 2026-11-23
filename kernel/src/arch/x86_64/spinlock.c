#include <spinlock.h>

void spinlockAcquire(Spinlock* lock) {
  while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
    asm("pause");
  }
}

void spinlockDrop(Spinlock* lock) {
  atomic_flag_clear_explicit(lock, memory_order_release);
}

void spinlockInit(Spinlock* lock) {
  *lock = (Spinlock)ATOMIC_FLAG_INIT;
}