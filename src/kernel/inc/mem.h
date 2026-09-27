#ifndef MEM_H
#define MEM_H

#include <stdatomic.h>
#include <util.h>

typedef atomic_flag Spinlock;

#define SPINLOCK(name) Spinlock name = ATOMIC_FLAG_INIT

void mSpinlockAcquire(Spinlock* lock);
void mSpinlockDrop(Spinlock* lock);
void mSpinlockInit(Spinlock* lock);

#endif