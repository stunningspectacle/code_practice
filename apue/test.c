#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#define to_string(x) #x

int main(int argc , char * argv[])
{
    int abc[] = {
	    [0] = 10, 
	    [1] = 20,
	    [2] = 30,
    };

        printf("%d\n", abc[2]);
    return 0;
}

