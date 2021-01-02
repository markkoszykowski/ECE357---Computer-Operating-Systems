#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include "spinlock.h"

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, Problem 2A

#define N_PROC 64
#define N_ITER 1e6

int main() {
	int *x;
	int status, MEM_SIZE;
	volatile char *lock;
	char *addr;
	pid_t pid;

	MEM_SIZE = sizeof(x) + sizeof(lock);

	if((addr = mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		fprintf(stderr, "Unable to allocate shared memory region: %s\n", strerror(errno));
		return -1;
	}
	
	x = (int*)addr;
	*x = 0;
	lock = addr + sizeof(x);
	*lock = 0;

	for(int i = 0; i < N_PROC; i++) {
		switch((pid = fork())) {
			case -1:
				fprintf(stderr, "Unable to fork new process: %s\n", strerror(errno));
				exit(-1);
			case 0:
				for(int j = 0; j < N_ITER; j++) {
					spin_lock(lock);
					*x += 1;
					spin_unlock(lock);
				}
				exit(0);
		}
	}

	for(int i = 0; i < N_PROC; i++) {
		if((pid = wait(&status)) == -1) {
			fprintf(stderr, "Unable to wait for child process: %s\n", strerror(errno));
			return -1;
		}
	}

	printf("Actual: %d\nIdeal: %d\n", *x, (int)(N_PROC * N_ITER));
	return 0;
}
