#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{

    printf("Supervisor started. PID: %d\n", getpid());

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        printf("Child process created. PID: %d\n", getpid());
    }
    else
    {
        printf("Worker process created. PID: %d\n", pid);

        wait(NULL);

        printf("Worker process finished.\n");
    }

    return 0;
}