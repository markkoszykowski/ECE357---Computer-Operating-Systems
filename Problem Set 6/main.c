#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include "fifo.h"

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, Problem 2D

#define N_PROC 2
#define N_ITER 1e4

int main() {
	struct fifo *f;
	int MEM_SIZE;

	MEM_SIZE = sizeof(*f);
	
	if((f = mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		fprintf(stderr, "Unable to allocate shared memory region: %s\n", strerror(errno));
		return -1;
	}

	fifo_init(f);
	for(int i = 0; i < N_PROC; i++) {
		switch(fork()) {
			case -1:
				fprintf(stderr, "Unable to fork new process: %s\n", strerror(errno));
				return -1;
			case 0:
				if(i == 0) {
					unsigned long d;
					for(int j = 0; j < ((N_PROC - 1) * N_ITER); j++) {
						d = fifo_rd(f);
						printf("%lu ", d);
					}
					printf("\n");
					return 0;
				}
				else {
					for(unsigned long j = 0; j < N_ITER; j++) {
						fifo_wr(f, ((i - 1) * 1e5 + j));
					}
					return 0;
				}
		}
	}

	while(wait(NULL) > 0) {}
	printf("All processes ended successfully!\n");
	return 0;
}
