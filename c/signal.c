#include <signal.h>
#include <stdio.h>


void sig_handler(int signum)
{
	printf("SIG %d received\n", signum);
}

void main()
{
	printf("pid = %d\n", getpid());

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	while (1) {
		sigsuspend(NULL);
	}
}

