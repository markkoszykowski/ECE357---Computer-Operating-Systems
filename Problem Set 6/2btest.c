#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include "sem.h"

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, Problem 2B

#define N_PROC 64
#define N_ITER 1e6

int main() {
	struct sem *s;
	int status, MEM_SIZE;
	int *x;
	pid_t pid;

	MEM_SIZE = sizeof(s) + sizeof(x);

	if((s = mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		fprintf(stderr, "Unable to allocate shared memory region: %s\n", strerror(errno));
		return -1;
	}

	x = s + sizeof(s);
	*x = 0;
	sem_init(s, 1);
	for(int i = 0; i < N_PROC; i++) {
		switch(fork()) {
			case -1:
				fprintf(stderr, "Unable to fork new process: %s\n", strerror(errno));
				exit(-1);
			case 0:
				//printf("%d, pid: %d\n", i, getpid());
				/*if(i < (N_PROC/2)) {
					for(int j = 0; j < N_ITER; j++) {
						sem_wait(s);
					}
				}
				else {
					for(int j = 0; j < N_ITER; j++) {
						sem_inc(s);
					}
				}
				*/
				for(int j = 0; j < N_ITER; j++) {
					sem_wait(s);
					*x += 1;
					sem_inc(s);
				}
				
				exit(0);
		}
	}

	for(int i = 0; i < N_PROC; i++) {
		if((pid = wait(&status)) == -1) {
			printf("Unable to wait.\n");
		}
	}
	printf("Actual x: %d\nIdeal x: %d\n", *x, (int)(N_PROC * N_ITER));
	printf("%d\n", s->count);
	return 0;
}
