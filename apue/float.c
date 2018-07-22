#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int main(int argc , char * argv[])
{
    float a;
    char *c, t;
    int i;

    printf("sizeof(float)=%d\n", sizeof(float));
    i = 20;
    printf("i = 0x%x\n", i);

    c = (char *)&a;
    a = 0.1;
    printf("0.1: 0x%x", *c++);
    for (i = 1; i < sizeof(float); i++) {
        printf(", 0x%x", *c++);
    }
    printf("\n");

    c = (char *)&a;
    a = 0.2;
    printf("0.2: 0x%x", *c);
    for (i = 1; i < sizeof(float); i++) {
        printf(", 0x%x", *c++);
    }
    printf("\n");
    return 0;
}

