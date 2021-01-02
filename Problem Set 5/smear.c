#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 5, Problem 3

int main(int argc, char *argv[]) {
	if(argc < 4) {
		fprintf(stderr, "USAGE: ./smear TARGET REPLACEMENT file1 {file2...}\n");
		return -1;
	}

	if(strlen(argv[1]) != strlen(argv[2])) {
		fprintf(stderr, "TARGET and REPLACEMENT should be of the same length\n");
		return -1;
	}

	char *addr;
	int fd;
	off_t size;

	for(int i = 3; i < argc; i++) {
		if((fd = open(argv[i], O_RDWR)) == -1) {
			fprintf(stderr, "Unable to open %s: %s\n", argv[i], strerror(errno));
			return -1;
		}

		if((size = lseek(fd, 0, SEEK_END)) == -1) {
			fprintf(stderr, "Unable to seek to end of %s: %s\n", argv[i], strerror(errno));
			return -1;
		}

		if((addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
			fprintf(stderr, "Unable to load %s into memory: %s\n", argv[i], strerror(errno));
			return -1;
		}

		for(char *j = addr; (j - addr) < (size - strlen(argv[1])); j++) {
			if(strncmp(j, argv[1], strlen(argv[1])) == 0) {
				memcpy(j, argv[2], strlen(argv[2]));
			}
		}

		if(msync(addr, size, MS_SYNC) == -1) {
			fprintf(stderr, "Unable to sync file %s with memory: %s\n", argv[i], strerror(errno));
			return -1;
		}

		if(munmap(addr, size) == -1) {
			fprintf(stderr, "Unable to unload %s from memory: %s\n", argv[i], strerror(errno));
			return -1;
		}

		if(close(fd) == -1) {
			fprintf(stderr, "Unable to close %s: %s\n", argv[i], strerror(errno));
			return -1;
		}
	}

	return 0;
}
