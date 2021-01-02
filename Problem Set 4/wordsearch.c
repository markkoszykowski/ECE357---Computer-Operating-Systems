#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 4, Problem 3

int totMatches = 0;

void handler(int signal) {
	fprintf(stderr, "pid %d recieved signal %d\n", getpid(), signal);
	fprintf(stderr, "Matched %d words\n", totMatches);
	exit(0);
}

int main(int argc, char *argv[]) {
	signal(SIGPIPE, handler);

	if(argc != 2) {
		fprintf(stderr, "USAGE: ./wordsearch FILE\n");
		return 1;
	}
	
	FILE *dictFile;
	if((dictFile = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Unable to open %s for reading: %s\n", argv[1], strerror(errno));
		return -1;
	}

	char **dict = NULL;
	char *word;
	int numWords = 0;
	ssize_t n;
	ssize_t len = 0;
	while((n = getline(&word, &len, dictFile)) != -1) {
		dict = (char**)realloc(dict, sizeof(char*) * (numWords + 1));
		dict[numWords] = (char*)malloc((strlen(word)+ 1) * sizeof(char));
		for(int i = 0; i < strlen(word); i++) {
			if(word[i] >= 'a' && word[i] <= 'z') {
				word[i] -= 32;
			}
		}
		strcpy(dict[numWords], word);
		numWords += 1;
	}
	fprintf(stderr, "Accepted %d words\n", numWords);
	
	while((n = getline(&word, &len, stdin)) != -1) {
		for(int i = 0; i < numWords; i++) {
			if(strcmp(word, dict[i]) == 0) {
				printf("%s", word);
				totMatches += 1;
			}
		}
	}

	fprintf(stderr, "Matched %d words\n", totMatches);
	return 0;
}
