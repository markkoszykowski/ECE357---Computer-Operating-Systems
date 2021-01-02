#include "fifo.h"

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, 2C

void fifo_init(struct fifo *f) {
	f->readInd = 0;
	f->writeInd = 0;
	sem_init(&f->mutex, 1);
	sem_init(&f->empty, 0);
	sem_init(&f->full, MYFIFO_BUFSIZ);
}

void fifo_wr(struct fifo *f, unsigned long d) {
	sem_wait(&f->full);
	while(!sem_try(&f->mutex)) {}
	f->f[f->writeInd++] = d;
	f->writeInd %= MYFIFO_BUFSIZ;
	sem_inc(&f->empty);
	sem_inc(&f->mutex);
}

unsigned long fifo_rd(struct fifo *f) {
	unsigned long d;

	sem_wait(&f->empty);
	while(!sem_try(&f->mutex)) {}
	d = f->f[f->readInd++];
	f->readInd %= MYFIFO_BUFSIZ;
	sem_inc(&f->full);
	sem_inc(&f->mutex);
	return d;
}
