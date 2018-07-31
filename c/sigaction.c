#include <stdio.h>
#include <signal.h>
#include <string.h>

void sig_handler(int signum)
{
	printf("SIG %d received\n", signum);
}

int main(void)
{
	int sig;
	struct sigaction sa;

	printf("pid = %d\n", getpid());
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sa.sa_handler = sig_handler;	
	for (sig = 0; sig < 32; sig++)
		sigaction(sig, &sa, NULL);

	sigprocmask(SIG_SETMASK, &sa.sa_mask, NULL);
	while (1) {
		sig = sigsuspend(&sa.sa_mask);
		printf("loop gets sig %d\n", sig);
	}
}
