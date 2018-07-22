#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <utime.h>

int main(int argc , char * argv[])
{
    int i, fd;
    struct stat statbuf;
    //struct utimbuf time;
    struct timespec time[2];

    for (i = 1; i < argc; i++) {
        if (stat(argv[i], &statbuf) == -1) {
            perror("Fail to fstat");
            continue;
        }
        if (fd = open(argv[i], O_RDWR | O_TRUNC) < 0) {
            printf("Fail to open %s: %s\n", argv[i], strerror(errno));
            continue;
        }
        time[0] = statbuf.st_atim;
        time[1] = statbuf.st_mtim;
        futimens(fd, time);
    }

    return 0;
}

