#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

char *name = "aaaaaaaaaabbbbbbb";
static void sig_int(int sig)
{
	printf("SIGINT\n%% ");
	fflush(stdout);
}

int main(void)
{
	pid_t pid;
	char buf[BUFSIZ];
	char *err_msg;

	printf("%s\n", name);
	if (signal(SIGINT, sig_int) == SIG_ERR)
		perror("signal error");
	printf("%% ");
	while (fgets(buf, sizeof(buf), stdin)) {
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0;
		if ((pid = fork()) < 0) {
			exit(-1);
		} else if (pid == 0) {
			execlp(buf, buf, (char *)0);
			exit(-1);
		}

		if (pid = waitpid(pid, (char *)0, 0) < 0)
			perror("waitpid error");
		printf("%% ");
	}
}


