#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

void hello(int a, int b, int c)
{
    printf("&a=0x%x, &b=0x%x, &c=0x%x\n", &a, &b, &c);

    int *t = &c;

    printf("t-1=0x%x\n", *(t-1));
    printf("t+1=0x%x\n", *(t+1));
}

int main(int argc , char * argv[])
{
    int i;

    printf("&main=0x%x\n", main);
    hello(1, 2, 3);

    return 0;
}

