#ifndef __SEM_H

#include <sys/types.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, Problem 2B

#define MAX 64

struct sem {
	volatile int count;
	volatile char lock;
	volatile pid_t queue[MAX]; 
};

void sem_init(struct sem *s, int count);
int sem_try(struct sem *s);
void sem_wait(struct sem *s);
void sem_inc(struct sem *s);

#define __SEM_H
#endif
