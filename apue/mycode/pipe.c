#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

char * cmd1[] = {"ls", "-l", 0};
char * cmd2[] = {"wc", "-l", 0};

int main(void)
{
    int pipefds[2];
    pid_t child1, child2;

    if (pipe(pipefds) == -1) {
        perror("pipe failed");
        return -1;
    }
    child1 = fork();
    if (!child1) {
        printf("This is child 1\n");
        close(STDIN_FILENO);
        if (dup2(pipefds[0], STDIN_FILENO) == -1) {
            perror("child1 dup2 failed");
            return -1;
        }
        close(pipefds[0]);
        close(pipefds[1]);
        execv("/usr/bin/wc", cmd2);
        printf("child 1, should not be here, something is wrong!\n");
    } else if (child1 < 0) {
        perror("fork child 1 failed");
        return -1;
    }
    close(pipefds[0]);

    child2 = fork();
    if (!child2) {
        printf("This is child 2\n");
        close(STDOUT_FILENO);
        if (dup2(pipefds[1], STDOUT_FILENO) == -1) {
            perror("child 2 dup2 failed");
            return -1;
        }
        close(pipefds[1]);
        execv("/bin/ls", cmd1);
        printf("child 2, should not be here, something is wrong!\n");
    } else if (child2 < 0) {
        perror("fork child 2 failed");
        return -1;
    }
    close(pipefds[1]);

    wait4(child2, NULL, 0, NULL);
    printf("Done!\n");

    return 0;
}

