#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <string.h>

#define PS_FILE "/dev/apds990x_psensor"
#define ALS_FILE "/dev/apds990x_lsensor"
#define LOG_FILE "/data/outlog"

int main(void)
{
	int ret, fd_ps, fd_als, val;
	char logstr[1024];
	time_t now;
	struct pollfd pollfds[2];

	fd_ps = open(PS_FILE, O_RDWR);
	if (fd_ps < 0) {
		printf("fd_ps: %s", strerror(fd_ps));
		return -1;
	}
	fd_als = open(ALS_FILE, O_RDWR);
	if (fd_als < 0) {
		printf("fd_als: %s", strerror(fd_als));
		return -1;
	}

	ret = ioctl(fd_ps, 0, 1);
	if (ret < 0) {
		printf("Enable ps sensor failed! code:%d\n", ret);
		return ret;
	}
	printf("Enable ps sensor!\n"); 

	ret = ioctl(fd_als, 0, 1);
	if (ret < 0) {
		printf("Enable als sensor failed! code:%d\n", ret);
		return ret;
	} 
	printf("Enable als sensor!\n"); 

	pollfds[0].fd = fd_ps;
	pollfds[0].events |= POLLIN;

	pollfds[1].fd = fd_als;
	pollfds[1].events |= POLLIN;

	for (;;) {
		poll(pollfds, 2, -1);
		if (pollfds[0].revents & POLLIN)
			read(fd_ps, &val, sizeof(val));
		else if (pollfds[1].revents & POLLIN)
			read(fd_als, &val, sizeof(val));
		else
			printf("Something Wrong!\n");
		time(&now);
		snprintf(logstr, sizeof(logstr), "val = %d, %s", val, asctime(localtime(&now)));
		printf("%s", logstr);
	}
}
	
