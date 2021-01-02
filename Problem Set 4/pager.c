#include <stdio.h>
#include <string.h>
#include <errno.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 4, Problem 3

int main() {
	char *line;
	ssize_t n;
	ssize_t len = 0;
	int i = 0;

	FILE *fp;
	if((fp = fopen("/dev/tty", "r")) == NULL) {
		fprintf(stderr, "Unable to open /dev/tty for reading: %s\n", strerror(errno));
		return -1;
	}

	while((n = getline(&line, &len, stdin)) != -1) {
		printf("%s", line);
		i += 1;
		if(i % 23 == 0) {
			char temp;
			printf("--- Press RETURN for more--- ");
			do {
				temp = getc(fp);
			} while(temp != 'q' && temp != 'Q' && temp != '\n');
			if(temp == 'q' || temp == 'Q') {
				fprintf(stderr, "*** Pager terminated by Q command ***\n");
				break;
			}
		}
	}

	return 0;
}
