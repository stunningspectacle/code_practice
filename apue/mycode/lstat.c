#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc , char * argv[])
{
    int i;
    char *ptr;
    struct stat stat;

    for (i = 1; i < argc; i++) {
        if (lstat(argv[i], &stat) == -1) {
            printf("Failed to lstat %s: %s\n", argv[i], strerror(errno));
            continue;
        }
        if (S_ISREG(stat.st_mode))
            ptr = "REG";
        else if (S_ISLNK(stat.st_mode))
            ptr = "LNK";
        else if (S_ISDIR(stat.st_mode))
            ptr = "DIR";
        else if (S_ISCHR(stat.st_mode))
            ptr = "CHR";
        else if (S_ISBLK(stat.st_mode))
            ptr = "BLK";
        else if (S_ISFIFO(stat.st_mode))
            ptr = "FIFO";
        else if (S_ISSOCK(stat.st_mode))
            ptr = "SOCK";
        else
            ptr = "NOT SUPPORT";

        printf("The type of %s is %s\n", argv[i], ptr);
	printf("The blksize is %ld\n", stat.st_blksize);
    }

    return 0;
}

