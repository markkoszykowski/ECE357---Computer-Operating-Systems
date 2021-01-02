#ifndef __SPINLOCK_H

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, Problem 2A

void spin_lock(volatile char *lock);
void spin_unlock(volatile char *lock);

#define __SPINLOCK_H
#endif
