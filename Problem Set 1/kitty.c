#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// Tamar Bacalu & Mark Koszykowski
// Problem Set 1, Problem 3

int main(int argc, char *argv[]) {
        // Define necessary variables
        int opt;
        char buf[4096];
        int stan_out = 1;
        int fd_out;
        int fd_in;
        int n, tot, temp;
        int bin = 0;
        int rs = 0;
        int ws = 0;
        int os = 0;

        // Loop through command line arguments
        while(((opt = getopt(argc, argv, ":o:")) != -1) || argc == 1 || optind != argc || (argc == 3 && optind == 3 && os == 1)) {
                // If branch to detect output file
                if(opt != -1) {
                        switch(opt) {
                                case 'o':
                                        stan_out = 0;
                                        if((fd_out = open(optarg, O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0) {
                                                fprintf(stderr, "Unable to open %s for writing: %s\n", optarg, strerror(errno));
                                                return -1;
                                        }
                                        os  = 1;
                                        break;
                                case '?':
                                        fprintf(stderr, "Unknown option: %c\n", optopt);
                                        return -1;
                                        break;
                        }
                }
                // Else branch to perform reads
                else{
                        tot = 0;
                        bin = 0;
                        // Check if reading from file or STDIN
                        if(argc == 1 || (argc == 3 && optind == 3 && os == 1) || *argv[optind] == '-') {
                                fd_in = STDIN_FILENO;
                        }
                        else {
                                if((fd_in = open(argv[optind], O_RDONLY)) < 0) {
                                        fprintf(stderr, "Unable to open %s for reading: %s\n", argv[optind], strerror(errno));
                                        return -1;
                                }
                        }
                        // Loop through contents of input & write them to output
                        while((n = read(fd_in, buf, sizeof(buf))) > 0) {
                                ++rs;
                                tot += n;
                                temp = n;
                                if(stan_out == 1) {
                                        fd_out = STDOUT_FILENO;
                                }
                                if((n = write(fd_out, buf, n)) != temp) {
                                        if(stan_out == 1) {
                                                fprintf(stderr, "Unable to write to <standard output>: %s\n", strerror(errno));
                                        }
                                        else {
                                                fprintf(stderr, "Unable to write to output file: %s\n", strerror(errno));
                                        }
                                        return -1;
                                }
                                if(!(isprint(buf[0]) || isspace(buf[0]))) {
                                        bin = 1;
                                }
                                ++ws;
                        }
                        // Check for read error
                        if(n < 0) {
                                if(fd_in == STDIN_FILENO) {
                                        fprintf(stderr, "Unable to read from <standard input>: %s\n", strerror(errno));
                                }
                                else {
                                        fprintf(stderr, "Unable to read from %s: %s\n", argv[optind], strerror(errno));
                                }
                                return -1;
                        }
                        // If 0 bytes are read, increase read count by not write count
                        else {
                                ++rs;
                        }
                        // Check for binary file
                        if(bin == 1) {
                                fprintf(stderr, "\nWarning! %s is a binary file!\n", argv[optind]);
                        }
                        // Output desired message depending on input
                        if(fd_in == STDIN_FILENO) {
                                fprintf(stderr, "\nNumber of bytes transferred from <standard input>: %d\nTotal number of read calls: %d\nTotal number of write calls: %d\n\n", tot, rs, ws);
                        }
                        else {
                                fprintf(stderr, "\nNumber of bytes transferred from %s: %d\nTotal number of read calls: %d\nTotal number of write calls: %d\n\n", argv[optind], tot, rs, ws);
                        }
                        // Close input file is there was one
                        if(fd_in != STDIN_FILENO) {
                                if((n = close(fd_in)) < 0) {
                                        fprintf(stderr, "Unable to close %s: %s\n", argv[optind], strerror(errno));
                                        return -1;
                                }
                        }
                        if(argc == 1 || (argc == 3 && optind == 3 && os == 1)) {
                                break;
                        }
                        ++optind;
                }
        }
        // Close output file if there was one
        if(stan_out == 0) {
                if((n = close(fd_out)) < 0) {
                        fprintf(stderr, "Unable to close output file: %s\n", strerror(errno));
                        return -1;
                }
        }
        return 0;
}
