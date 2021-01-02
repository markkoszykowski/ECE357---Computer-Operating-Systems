#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 3, Problem 3

int exitCode = 0;

void runCommand(int argNum, char *cmdArgs[], int redirNum, char *cmdRedir[]) {
        pid_t pid;
        struct timeval start;
        struct timeval end;
        struct rusage ru;
        int status;

        if(gettimeofday(&start, NULL) == -1) {
                fprintf(stderr, "*ERROR* failed to obtain real time: %s\n", strerror(errno));
                exitCode = -1;
                return;
        }

        if(strcmp(cmdArgs[0], "cd") == 0) {
                char *dir;
                if(cmdArgs[1]) {
                        dir = cmdArgs[1];
                }
                else {
                        dir = getenv("HOME");
                }
                if(chdir(dir) == -1) {
                        fprintf(stderr, "%s: %s: %s\n", cmdArgs[0], dir, strerror(errno));
                        exitCode = -1;
                        return;
                }
                return;
        }

        if(strcmp(cmdArgs[0], "pwd") == 0) {
                char buf[4096];
                if(getcwd(buf, sizeof(buf)) == NULL) {
                        fprintf(stderr, "%s: %s\n", cmdArgs[0], strerror(errno));
                        exitCode = -1;
                        return;
                }
                printf("%s\n", buf);
                return;
        }

        if(strcmp(cmdArgs[0], "exit") == 0) {
                if(cmdArgs[1]) {
                        exitCode = atoi(cmdArgs[1]);
                }
                exitCode = exitCode&255;
                printf("exiting shell with exit code %d\n", exitCode);
                exit(exitCode);
        }

        switch((pid = fork())) {
                case -1:
                        fprintf(stderr, "*ERROR* failed to fork to run %s: %s\n", cmdArgs[0], strerror(errno));
                        exitCode = -1;
                        return;
                case 0:
                        if(redirNum > 0) {
                                int fd;
                                char *fileName;
                                for(int i = 0; i < redirNum; i++) {
                                        if(cmdRedir[i][0] == '<') {
                                                fileName = cmdRedir[i] + 1;
                                                if((fd = open(fileName, O_RDONLY)) == -1) {
                                                        fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                        exitCode = 1;
                                                        exit(exitCode);
                                                }
                                                if(dup2(fd, STDIN_FILENO) == -1) {
                                                        fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                        exitCode = 1;
                                                        exit(exitCode);
                                                }
                                                if(close(fd) == -1) {
                                                        fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                        exit(exitCode);
                                                }
                                        }
                                        else if(cmdRedir[i][0] == '>') {
                                                if(cmdRedir[i][1] == '>') {
                                                        fileName = cmdRedir[i] + 2;
                                                        if((fd = open(fileName, O_WRONLY|O_CREAT|O_APPEND, 0666)) == -1) {
                                                                fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                                exitCode = 1;
                                                                exit(exitCode);
                                                        }
                                                }
                                                else {
                                                        fileName = cmdRedir[i] + 1;
                                                        if((fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1) {
                                                                fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                                exitCode = 1;
                                                                exit(exitCode);
                                                        }
                                                }
                                                if(dup2(fd, STDOUT_FILENO) == -1) {
                                                        fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                        exitCode = 1;
                                                        exit(exitCode);
                                                }
                                                if(close(fd) == -1) {
                                                        fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                        exitCode = 1;
                                                        exit(exitCode);
                                                }
                                        }
                                        else {
                                                if(cmdRedir[i][2] == '>') {
                                                        fileName = cmdRedir[i] + 3;
                                                        if((fd = open(fileName, O_WRONLY|O_CREAT|O_APPEND, 0666)) == -1) {
                                                                fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                                exitCode = 1;
                                                                exit(exitCode);
                                                        }
                                                }
                                                else {
                                                        fileName = cmdRedir[i] + 2;
                                                        if((fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1) {
                                                                fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                                exitCode = 1;
                                                                exit(exitCode);
                                                        }
                                                }
                                                if(dup2(fd, STDERR_FILENO) == -1) {
                                                        fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                        exitCode = 1;
                                                        exit(exitCode);
                                                }
                                                if(close(fd) == -1) {
                                                        fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
                                                        exitCode = 1;
                                                        exit(exitCode);
                                                }
                                        }
                                }
                        }

                        if(execvp(cmdArgs[0], cmdArgs) == -1) {
                                fprintf(stderr, "%s: %s\n", cmdArgs[0], strerror(errno));
                                exitCode = 127;
                                exit(exitCode);
                        }
                default:
                        if(wait3(&status, 0, &ru) == -1) {
                                fprintf(stderr, "*ERROR* failed to wait for child process: %s\n", strerror(errno));
                                exitCode = -1;
                                return;
                        }

                        if(status != 0) {
                                if(WIFSIGNALED(status)) {
                                        fprintf(stderr, "Child process %d exited with signal %d\n", pid, WTERMSIG(status));
                                        exitCode = 128 + WTERMSIG(status);
                                }
                                else {
                                        fprintf(stderr, "Child process %d exited with return value %d\n", pid, WEXITSTATUS(status));
                                        exitCode = WEXITSTATUS(status);
                                }
                        }
                        else {
                                fprintf(stderr, "Child process %d exited normally\n", pid);
                                exitCode = 0;
                        }

                        if(gettimeofday(&end, NULL) == -1) {
                                fprintf(stderr, "*ERROR* failed to obtain real time: %s\n", strerror(errno));
                                exitCode = -1;
                                return;
                        }

                        fprintf(stderr, "Real: %ld.%03ds User: %ld.%03ds Sys: %ld.%03ds\n", abs(end.tv_sec - start.tv_sec), abs(end.tv_usec - start.tv_usec), ru.ru_utime.tv_sec, ru.ru_utime.tv_usec, ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);

                        return;
        }
        return;
}

int main() {
        char *line = NULL;
        size_t len = 0;
        ssize_t nread;

        while((nread = getline(&line, &len, stdin)) != -1) {
                if(line[0] == '#') {
                        continue;
                }
                
                line[strlen(line) - 1] = 0;

                char **cmdArgs = NULL;
                char **cmdRedir = NULL;
                int argNum = 0;
                int redirNum = 0;

                char *word = strtok(line, " \t");
                while(word != NULL) {
                        if(word[0] == '<' || word[0] == '>' || (word[0] == '2' && word[1] == '>')) {
                                redirNum += 1;
                                cmdRedir = (char**)realloc(cmdRedir, sizeof(char*) * redirNum);
                                if(cmdRedir == NULL) {
                                        fprintf(stderr, "Failed to allocate space for I/O redirection tokens.\n");
                                        break;
                                }
                                cmdRedir[redirNum - 1] = word;
                        }
                        else {
                                argNum += 1;
                                cmdArgs = (char**)realloc(cmdArgs, sizeof(char*) * (argNum + 1));
                                if(cmdArgs == NULL) {
                                        fprintf(stderr, "Failed to allocate space for command and argument tokens.\n");
                                        break;
                                }
                                cmdArgs[argNum - 1] = word;
                        }
                        word = strtok(NULL, "  \t");
                }

                if(argNum > 0) {
                        cmdArgs[argNum] = NULL;
                        runCommand(argNum, cmdArgs, redirNum, cmdRedir);
                }
        }

        exitCode = exitCode&255;
        printf("end of file read, exiting shell with exit code %d\n", exitCode);

        return exitCode;
}
