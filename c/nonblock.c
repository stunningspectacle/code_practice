#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	char *path;
	int fd;
	char buf[128];
	int len;

	if (argc != 2)
		return -1;
	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		perror(argv[1]);
		return -1;
	}

	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	memset(buf, 0, sizeof(buf));
	len = read(fd, buf, sizeof(buf));
	if (len >= 0)
		printf("get %d bytes:%s\n", len, buf);
	else
		printf("no data: len(%d), errno(%d), %s\n",
				len, errno, strerror(errno));

	close(fd);

	return 0;
}
	
