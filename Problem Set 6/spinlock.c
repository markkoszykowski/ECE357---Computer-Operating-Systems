#include "tas.h"
#include "spinlock.h"
#include <sched.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, Problem 2A

void spin_lock(volatile char *lock) {
	while(tas(lock) != 0) {
		sched_yield();
	}
}

void spin_unlock(volatile char *lock) {
	*lock = 0;
}
