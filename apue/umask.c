#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define RWXRWXRWX (S_IRWXU | S_IRWXG | S_IRWXO)

int main(int argc, char *argv[])
{
    open("test1", O_CREAT, RWXRWXRWX);

    umask(S_IWUSR | S_IWGRP | S_IWOTH);

    open("test2", O_CREAT, RWXRWXRWX);
}

