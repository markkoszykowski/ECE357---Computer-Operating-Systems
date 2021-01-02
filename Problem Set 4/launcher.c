#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 4, Problem 3

int fds[2][2];
char pipeOps[2][6] = {"read", "write"};

void spawnChild(int inFd, int outFd, char *cmdArgs[]) {
	switch(fork()) {
		case -1:
			fprintf(stderr, "Unable to fork new process for %s: %s\n", cmdArgs[0], strerror(errno));
			exit(-1);
		case 0:
			if(inFd != -1) {
				if(dup2(inFd, STDIN_FILENO) == -1) {
					fprintf(stderr, "Unable to redirect stdin for %s: %s\n", cmdArgs[0], strerror(errno));
					exit(-1);
				}
			}
			if(outFd != -1) {
				if(dup2(outFd, STDOUT_FILENO) == -1) {
					fprintf(stderr, "Unable to redirect stdout for %s: %s\n", cmdArgs[0], strerror(errno));
					exit(-1);
				}
			}
			for(int i = 0; i < 2; i++) {
				for(int j = 0; j < 2; j++) {
					if(close(fds[i][j]) == -1) {
						fprintf(stderr, "Unable to close %s end for pipe %d in child: %s\n", pipeOps[j], (i+1), strerror(errno));
						exit(-1);
					}
				}
			}
			if(execvp(cmdArgs[0], cmdArgs) == -1) {
				fprintf(stderr, "%s: %s\n", cmdArgs[0], strerror(errno));
				exit(127);
			}
	}
}

int main(int argc, char *argv[]) {
	char commands[3][13] = {"./wordgen", "./wordsearch", "./pager"};
	char dict[] = "words.txt";
	char *numWords = argv[1];

	if(argc > 2) {
		fprintf(stderr, "USAGE: ./launcher [OPTION]\n");
		return 1;
	}

	for(int i = 0; i < 2; i++) {
		if(pipe(fds[i]) != 0) {
			fprintf(stderr, "Unable to create pipe %d: %s\n", (i+1), strerror(errno));
			return -1;
		}
	}

	char **cmdArgs = (char**)malloc(3 * sizeof(char*));

	cmdArgs[0] = commands[0];
	cmdArgs[1] = numWords;
	cmdArgs[2] = NULL;
	spawnChild(-1, fds[0][1], cmdArgs);

	cmdArgs[0] = commands[1];
	cmdArgs[1] = dict;
	cmdArgs[2] = NULL;
	spawnChild(fds[0][0], fds[1][1], cmdArgs);

	cmdArgs[0] = commands[2];
	cmdArgs[1] = NULL;
	spawnChild(fds[1][0], -1, cmdArgs);
	
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < 2; j++) {
			if(close(fds[i][j]) == -1) {
				fprintf(stderr, "Unable to close %s end of pipe %d in parent: %s\n", pipeOps[j], (i+1), strerror(errno));
				return -1;
			}
		}
	}
				

	pid_t cpid;
	unsigned int status;

	for(int i = 0; i < 3; i++) {
		if((cpid = wait(&status)) == -1) {
			fprintf(stderr, "Error waiting for child process %d: %s\n", cpid, strerror(errno));
			return -1;
		}
		if(WIFSIGNALED(status)) {
			if(WCOREDUMP(status)) {
				status = WTERMSIG(status) + 128;
			}
			else {
				status = WTERMSIG(status);
			}
		}
		else {
			status = WEXITSTATUS(status);
		}
		fprintf(stderr, "Child %d exited with %d\n", cpid, status);
	}
	return 0;
}
