#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main()
{ 
	char s[1024*4];
	int fd, num = 0;
	int i;
	fd = open("new.fw", O_RDWR);
	if (fd < 0) {
		printf("open file failed, %s\n", strerror(errno));
		return -1;
	}
	num = read(fd, s, sizeof(s));
	printf("num=%d\n", num);
	for (i = 50; i > 0; i--)
		printf("0x%x: %c\n", s[num - i], s[num - i]);

	return 0;
}
