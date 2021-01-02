#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <libgen.h>

// ECE357 - OS Project 2
// Tamar Bacalu & Mark Koszykowski

// Define function to either check if inode with >1 link has already been encountered or to record it
int checkInode(int inodeNum, int *seenInodes) {
        for(int i = 0; i < 4096; i++) {
                // Inode has already been encountered
                if(seenInodes[i] == inodeNum) {
                        return -1;
                }
                // Inode hasnt been encountered so recorded
                else if(seenInodes[i] == 0) {
                        seenInodes[i] = inodeNum;
                        return 0;
                }
        }
        // More than 4096 inodes with >1 links (unlikely)
        fprintf(stderr, "*NOTE* Array containing inodes with >1 links has been filled. All further inodes with >1 links will be accounted for more than once.\n");
        return 0;
}

// Define function to update the statistics asked for (function should prevent double counting of inodes with hardlinks)
void updateCounts(char *fulldir, char *name, struct stat st, int *inodes, int *partsBtoE, int *seenInodes) {
        int problem = 0;
        int check;
        // Update the necessary statistics depending on inode type
        switch((st.st_mode&S_IFMT)) {
                case S_IFIFO:
                        if(st.st_nlink > 1) {
                                partsBtoE[2] += 1;
                                if((check = checkInode(st.st_ino, seenInodes)) == -1) {
                                        partsBtoE[2] += check;
                                        break;
                                }
                        }
                        inodes[0] += 1;
                        break;
                case S_IFCHR:
                        if(st.st_nlink > 1) {
                                partsBtoE[2] += 1;
                                if((check = checkInode(st.st_ino, seenInodes)) == -1) {
                                        partsBtoE[2] += check;
                                        break;
                                }
                        }
                        inodes[1] += 1;
                        break;
                case S_IFDIR:
                        inodes[2] += 1;
                        // If its a directory check for 'problematic' name
                        for(int i = 0; i < strlen(name); i++) {
                                char c = name[i];
                                // Source for forbidden characters:
                                // https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
                                // Only valid typable ASCII characters allowed
                                if(!(isprint(c) || isspace(c)) || c == '/') {
                                        problem = 1;
                                        break;
                                }
                        }
                        if(problem) {
                                partsBtoE[4] += 1;
                        }
                        break;
                case S_IFBLK:
                        if(st.st_nlink > 1) {
                                partsBtoE[2] += 1;
                                if((check = checkInode(st.st_ino, seenInodes)) == -1) {
                                        partsBtoE[2] += check;
                                        break;
                                }
                        }
                        inodes[3] += 1;
                        break;
                case S_IFREG:
                        if(st.st_nlink > 1) {
                                partsBtoE[2] += 1;
                                if((check = checkInode(st.st_ino, seenInodes)) == -1) {
                                        partsBtoE[2] += check;
                                        break;
                                }
                        }
                        inodes[4] += 1;
                        // Record the size of the regular file and number of blocks it occupies
                        partsBtoE[0] += st.st_size;
                        partsBtoE[1] += st.st_blocks;
                        break;
                case S_IFLNK:
                        if(st.st_nlink > 1) {
                                partsBtoE[2] += 1;
                                if((check = checkInode(st.st_ino, seenInodes)) == -1) {
                                        partsBtoE[2] += check;
                                        break;
                                }
                        }
                        // Check if symlink has valid target
                        if((check = access(fulldir, F_OK)) != 0 && errno == ENOENT) {
                                partsBtoE[3] += 1;
                        }
                        inodes[5] += 1;
                        break;
                case S_IFSOCK:
                        if(st.st_nlink > 1) {
                                partsBtoE[2] += 1;
                                if((check = checkInode(st.st_ino, seenInodes)) == -1) {
                                        partsBtoE[2] += check;
                                        break;
                                }
                        }
                        inodes[6] += 1;
                        break;
                default:
                        // Record the non universal type of inode
                        if(st.st_nlink > 1) {
                                partsBtoE[2] += 1;
                                if((check = checkInode(st.st_ino, seenInodes)) == -1) {
                                        partsBtoE[2] += check;
                                        break;
                                }
                        }
                        inodes[7] += 1;
        }
}

// Define a recursive function to iterate through the initially given directory
void readDir(char *dirName, int *inodes, int *partsBtoE, int *seenInodes) {
        DIR *dirp;
        struct dirent *de;
        struct stat st;
        char fulldir[4096];
        char temp[4096];

        strcpy(temp, dirName);
        strcat(temp, "/");
        
        // If the directory cant be opened, return and go to next directory
        if(!(dirp = opendir(dirName))) {
                fprintf(stderr, "Can not open directory %s: %s\n", dirName, strerror(errno));
                return;
        }
        errno = 0;
        // Read through contents of directory
        while(de = readdir(dirp)) {
                // Store the full path name
                strcpy(fulldir, temp);
                strcat(fulldir, de->d_name);
                // If meta data cannot be retrieved, skip to next object in directory
                if(stat(fulldir, &st)) {
                        fprintf(stderr, "Error retrieving metadata from %s: %s\n", fulldir, strerror(errno));
                        errno = 0;
                        continue;
                }
                // Check if object is not . or .. (will cause infinite loop) or proc (easiest way to prevent issues with folder)
                if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && strcmp(de->d_name, "proc") != 0) {
                        updateCounts(fulldir, de->d_name, st, inodes, partsBtoE, seenInodes);
                        // If theres a subdirectory, recursively call function
                        if((st.st_mode&S_IFMT) == S_IFDIR) {
                                readDir(fulldir, inodes, partsBtoE, seenInodes);
                        }
                }
        }
        if(errno) {
                fprintf(stderr, "Error reading directory %s: %s\n", dirName, strerror(errno));
        }
        closedir(dirp);
        return;
}

int main(int argc, char *argv[]) {
        int inodes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        char types[7][9] = {"S_IFIFO", "S_IFCHR", "S_IFDIR", "S_IFBLK", "S_IFREG", "S_IFLNK", "S_IFSOCK"};
        int partsBtoE[5] = {0, 0, 0, 0, 0};
        int seenInodes[4096];
        char *name;

        // Initialize array of seen inodes to all zeros since zero is a reserved inode value
        for(int i = 0; i < 4096; i++) {
                seenInodes[i] = 0;
        }

        // Make sure a relative pathname is given
        if(argc == 1) {
                fprintf(stderr, "No directory given.\n");
                return -1;
        }

        struct stat st;
        if(stat(argv[1], &st)) {
                fprintf(stderr, "Error retrieving metadata from %s: %s\n", argv[1], strerror(errno));
                return -1;
        }
        
        // Extract name of given directory from provided path
        name = basename(argv[1]);        

        // Update statistics based on provided location
        updateCounts(argv[1], name, st, inodes, partsBtoE, seenInodes);
        // If path is to a directory then read through it
        if((st.st_mode&S_IFMT) == S_IFDIR) {
                readDir(argv[1], inodes, partsBtoE, seenInodes);
        }
        
        // Print out the asked for statistics
        printf("\n*NOTE* Multiple instances of objects with the same inode will only be accounted for ONCE\n\n");

        for(int i = 0; i < 7; i++) {
                printf("Number of inodes of type %s encountered: %d\n", types[i], inodes[i]);
        }
        printf("Number of inodes not of the seven universal types encountered: %d\n", inodes[7]);

        printf("\nSum of regular file sizes: %d\n", partsBtoE[0]);
        printf("Sum of disk blocks allocated for regular files: %d\n", partsBtoE[1]);

        printf("\nNumber of inodes (except directories) that have a link count >1: %d\n", partsBtoE[2]);

        printf("\nNumber of symlinks encountered without valid target: %d\n", partsBtoE[3]);

        printf("\nNumber of directories with 'problematic' name: %d\n", partsBtoE[4]);

        return 0;
}
