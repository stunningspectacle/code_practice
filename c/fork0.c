#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	pid_t pid;
	char *argv[] = {"/bin/echo", "aaaaaaaa", "bbb"};

	printf("111111111\n");
	if (!(pid = fork())) {
		printf("222222222\n");
		sleep(5);
		execv("/bin/echo", argv);
		printf("3333333\n");
	} else {
		printf("I'm waiting\n");
		wait4(pid, NULL, 0, NULL);
		printf("I'm exiting\n");
		exit(EXIT_SUCCESS);
	}

	return 0;
}
