#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int main(int argc , char * argv[])
{
    int fd, len;
    char buf[BUFSIZ];

    if ((fd = open(".", O_RDONLY)) < 0) {
        perror("Failed to open dir");
        return -1;
    }

    if ((len = read(fd, buf, BUFSIZ)) < 0) {
        perror("Failed to read dir");
        return -1;
    }

    printf("len=%d, ok:%s\n", len, buf);

    return 0;
}

