#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdatomic.h>
#include <util.h>

typedef atomic_flag Spinlock;

#define SPINLOCK(name) Spinlock name = ATOMIC_FLAG_INIT

void spinlockAcquire(Spinlock* lock);
void spinlockDrop(Spinlock* lock);
void spinlockInit(Spinlock* lock);

#endif