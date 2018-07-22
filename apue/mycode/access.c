#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s file\n", argv[0]);
        return 0;
    }
    if (access(argv[1], R_OK) == -1)
        perror("access R_OK test failed");
    else
        printf("access R_OK test passed\n");

    
    if (open(argv[1], O_RDONLY) < 0)
        perror("open with O_RDONLY failed");
    else
        printf("open with O_RDONLY passed\n");
}

