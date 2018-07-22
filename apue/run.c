#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

char buf1[] = "abcdefghijk";
char buf2[] = "ABCDEFGHIJK";

void get_flags(int fd)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) < 0) {
        perror("fcntl failed");
        return;
    }
    printf("fd %d:", fd);
    switch (flags & O_ACCMODE) {
        case O_RDONLY:
            printf("read only");
            break;
        case O_WRONLY:
            printf("write only");
            break;
        case O_RDWR:
            printf("read write");
            break;
    }
    if (flags & O_TRUNC)
        printf(", truncate");
    if (flags & O_APPEND)
        printf(", append");
    if (flags & O_NONBLOCK)
        printf(", nonblocking");
    if (flags & O_SYNC)
        printf(", sync writes");
#if !defined(_POSIX_C_SOURCE) && defined(O_FSYNC) && (O_FSYNC != O_SYNC)
    if (flags & O_FSYNC)
        printf(", fsync write");
#endif
    putchar('\n');
}

int main(int argc, char *argv[])
{
    int i, fd;
    int value;
    off_t offset;
    char buf[20];

    if ((fd = open("abc", O_RDWR | O_CREAT | O_APPEND)) < 0) {
        perror("open file failed");
        return 0;
    }

    if ((offset = lseek(fd, 1, SEEK_SET ))== -1) {
        perror("lseek file failed");
        return 0;
    } else {
        printf("offset = %ld\n", offset);
    }
    if (read(fd, buf, sizeof(buf)) < 0) {
        perror("lseek file failed");
        return 0;
    } else {
        buf[sizeof(buf) - 1] = '\0';
        printf("read: %s\n", buf);
    }
    if (write(fd, "xx", 2) != 2) {
        perror("write file failed");
        return 0;
    }

    return 0;
}

