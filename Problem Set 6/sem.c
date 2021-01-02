#include "spinlock.h"
#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 6, 2B

void handler(int signal) {}

void sem_init(struct sem *s, int count) {
	s->count = count;
	s->lock = 0;
	for(int i = 0; i < MAX; i++) {
		s->queue[i] = -1;
	}
	if(signal(SIGUSR1, handler) == SIG_ERR) {
		fprintf(stderr, "Failed to set behaviour of SIGUSR1: %s\n", strerror(errno));
		exit(-1);
	}
}

int sem_try(struct sem *s) {
	spin_lock(&s->lock);
	if(s->count > 0) {
		s->count -= 1;
		spin_unlock(&s->lock);
		return 1;
	}
	spin_unlock(&s->lock);
	return 0;
}

void sem_wait(struct sem *s) {
	sigset_t emptySet, blockSet;
	
	if(sigemptyset(&emptySet) == -1) {
		fprintf(stderr, "Failed to create empty signal mask: %s\n", strerror(errno));
		exit(-1);
	}
	if(sigemptyset(&blockSet) == -1) {
		fprintf(stderr, "Failed to empty blocking signal mask: %s\n", strerror(errno));
		exit(-1);
	}
	if(sigaddset(&blockSet, SIGUSR1) == -1) {
		fprintf(stderr, "Failed to block SIGUSR1 in blocking signal mask: %s\n", strerror(errno));
		exit(-1);
	}
	
	while(!sem_try(s)) {
		spin_lock(&s->lock);
		if(sigprocmask(SIG_BLOCK, &blockSet, NULL) == -1) {
			fprintf(stderr, "Failed to change signal mask: %s\n", strerror(errno));
			exit(-1);
		}
		if(s->queue[MAX - 1] != -1) {
			spin_unlock(&s->lock);
			fprintf(stderr, "Wait list full, pid: %d.\n", getpid());
			exit(-1);
		}
		
		for(int i = 0; i < MAX; i++) {
			if(s->queue[i] == -1) {
				s->queue[i] = getpid();
				break;
			}
		}
		spin_unlock(&s->lock);
		sigsuspend(&emptySet);
	}
}

void sem_inc(struct sem *s) {
	spin_lock(&s->lock);
	s->count += 1;
	for(int i = 0; i < MAX; i++) {
		if(s->queue[i] != -1) {
			if(kill(s->queue[i], SIGUSR1) == -1) {
				fprintf(stderr, "Unable to send SIGUSR1 to pid %d: %s\n", s->queue[i], strerror(errno));
				exit(-1);
			}
			s->queue[i] = -1;
		}
		else {
			break;
		}
	}
	spin_unlock(&s->lock);
}
