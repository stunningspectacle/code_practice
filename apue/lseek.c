#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

char *buf1 = "12345";
char *buf2 = "abcde";

void copy(int fd)
{
	int fd1, len;
	char buf[BUFSIZ];
	off_t pre_offset, cur_offset;

	if ((fd1 = open("copy.txt", O_CREAT | O_RDWR | O_TRUNC)) < 0) {
		perror("open failed");
		return;
	}

	pre_offset = cur_offset = 0;
	while ((lseek(fd, , BUFSIZ)) > 0)
		write(fd1, buf, len);
}

int main(int argc, char *argv[])
{
	int fd;

	if ((fd = open("holefile.txt", O_CREAT | O_RDWR)) < 0) {
		perror("open failed");
		return 0;
	}
	copy(fd);
	/*
	write(fd, buf1, strlen(buf1));
	if (lseek(fd, 20, SEEK_CUR) == -1) {
		perror("lseek failed");
		return 0;
	}
	write(fd, buf2, strlen(buf2));
	*/
}

