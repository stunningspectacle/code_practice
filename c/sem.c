#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define SEMKEY	0xdeadbeea
#define PERMS	0666

struct sembuf op_down = { 0, -1, 0 };
struct sembuf op_up = { 0, 1, 0 };

int semid = -1;
int res;

void init_sem()
{
	semid = semget(SEMKEY, 0, IPC_CREAT | PERMS);
	if (semid < 0) {
		perror(NULL);
		semid = semget(SEMKEY, 1, IPC_CREAT | PERMS);
		if (semid < 0) {
			perror("Cannot create semaphore:");
			exit(-1);
		}
		semctl(semid, 0, SETVAL, 1);
	}
}

void down()
{
	semop(semid, &op_down, 1);
}

void up()
{
	semop(semid, &op_up, 1);
}

int main()
{
	int count = 0;
	pid_t pid = getpid();

	init_sem();
	while (1) {
		printf("%d: before critical region\n", pid);
		down();

		count++;
		printf("%d: in critical region, count=%d\n", getpid(), count);
		sleep(5);

		up();
		printf("%d: exit critical region\n", pid);
	}

	return 0;
}

	
