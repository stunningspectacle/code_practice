#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int main(int argc , char * argv[])
{
    int i, fd;

    fd = open("abc", O_RDWR);
    readlink

    unlink("abc");

    sleep(10);

    return 0;
}

