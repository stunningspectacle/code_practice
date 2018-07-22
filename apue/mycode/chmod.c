#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define RWXRWXRWX (S_IRWXU | S_IRWXG | S_IRWXO)

int main(int argc, char *argv[])
{
    struct stat st;

    umask(0);
    if (stat("test1", &st) == -1) {
        perror("stat failed");
        return 0;
    }
    if (chmod("test1", (st.st_mode & ~(S_IWUSR | S_IWGRP | S_IWOTH)) | S_ISUID) == -1) {
        perror("chmod failed");
        return 0;
    }

    if (stat("test2", &st) == -1) {
        perror("stat failed");
        return 0;
    }
    if (chmod("test2", st.st_mode | S_IWUSR | S_IWGRP | S_IWOTH) == -1) {
        perror("chmod failed");
        return 0;
    }
        
}

