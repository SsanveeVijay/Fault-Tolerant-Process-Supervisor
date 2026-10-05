#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_WORKERS 3

int main() {

    pid_t workers[NUM_WORKERS];

    printf("Supervisor started. PID: %d\n\n", getpid());

    for (int i = 0; i < NUM_WORKERS; i++) {

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            return 1;
        }

        if (pid == 0) {

            printf("Starting Worker %d...\n", i + 1);

            execl("./worker", "worker", NULL);

            perror("exec failed");
            return 1;
        }

        workers[i] = pid;

        printf("Worker %d created - PID: %d\n",
               i + 1, workers[i]);
    }

    printf("\nAll workers created.\n");
    printf("Supervisor is monitoring the workers.\n\n");

    while (1) {
        wait(NULL);
    }

    return 0;
}