#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 4, Problem 3

int main(int argc, char *argv[]) {
	int numWords = 0;
	
	if(argc > 2) {
		fprintf(stderr, "USAGE: ./wordgen [OPTION]\n");
		return 1;
	}
	else if(argc == 2) {
		numWords = atoi(argv[1]);
	}
	srand(time(0));

	char *word;
	int i = 0;
	int wordLen = 0;
	char temp;
	while(1) {
		do {
			wordLen = rand() % 16;
		} while(wordLen < 3);
		
		word = (char*)malloc((wordLen + 1) * sizeof(char));

		for(int j = 0; j < wordLen; j++) {
			word[j] = (rand() % 26) + 65;
		}
		word[wordLen] = '\n';
		printf("%s", word);
		if(numWords != 0) {
			i += 1;
			if(i == numWords) {
				break;
			}
		}

	}
	fprintf(stderr, "Finished generating %d candidate words\n", numWords);
	return 0;
}
