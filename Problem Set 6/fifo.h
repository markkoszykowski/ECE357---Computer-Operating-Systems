#ifndef __FIFO_H

#include "sem.h"

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, Problem 2C

#define MYFIFO_BUFSIZ 4096

struct fifo {
	int readInd, writeInd;
	unsigned long f[MYFIFO_BUFSIZ];
	struct sem mutex, empty, full;
};

void fifo_init(struct fifo *f);
void fifo_wr(struct fifo *f, unsigned long d);
unsigned long fifo_rd(struct fifo *f);

#define __FIFO_H
#endif
