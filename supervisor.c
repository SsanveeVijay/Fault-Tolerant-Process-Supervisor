#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    printf("Supervisor started. PID: %d\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {

        printf("Starting worker process...\n");

        execl("./worker", "worker", NULL);

        perror("exec failed");
        return 1;
    }
    else {

        printf("Worker process created. PID: %d\n", pid);

        wait(NULL);

        printf("Worker process finished.\n");
    }

    return 0;
}