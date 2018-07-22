#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <linux/fs.h>
#include <string.h>

char *s = "12345678900987654321";
void main()
{
	int i, fd;
	void *buf;

	fd = open("mmap.txt", O_RDWR | O_CREAT);
	if (fd < 0) {
		perror(strerror(errno));
		return;
	}

	buf = (char *)mmap(0, BLOCK_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	//buf = (char *)mmap(0, BLOCK_SIZE, PROT_WRITE , MAP_PRIVATE, fd, 0);
	if (buf < 0) {
		perror(strerror(errno));
		return;
	} else {
		printf("buf: %p\n", buf);
	}

	for (i = 0; i < 10; i++)
		printf("%c", ((char *)buf)[i]);
	printf("\n");

	memcpy(buf, s, strlen(s));

	for (i = 0; i < 10; i++)
		printf("%c", ((char *)buf)[i]);
	printf("\n");

	if (munmap(buf, BLOCK_SIZE)) {
		perror(strerror(errno));
	}

	close(fd);
}

	

