#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

int myfork(void)
{
	pid_t p;
	int fd;

	if ((p = fork()) > 0)
		exit(EXIT_SUCCESS);
	else {
		fd = open("/dev/ltr502als_psensor", O_RDWR);
		ioctl(fd, 0, 1);
		for (;;)
			sleep(100);
	}
}

