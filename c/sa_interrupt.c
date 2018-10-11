#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int signaled;

void sig_handler(int signum)
{
	printf("%d signal called\n", signum);
	signaled = 1;
}

int main(void)
{
	char ch;
	struct sigaction sigact;
	sigact.sa_handler = sig_handler;
	/* SA_RESTART: if one syscall is interrupted by this signal,
	 * the syscall will be re-executed after signal handler */
	//sigact.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sigact, NULL);

	while (read(STDIN_FILENO, &ch, 1) != 1 && !signaled);

	return 0;
}

