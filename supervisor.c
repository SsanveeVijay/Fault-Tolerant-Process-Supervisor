#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_WORKERS 3

struct Worker {
    int id;
    pid_t pid;
    int active;
};

int main() {

    struct Worker workers[NUM_WORKERS];

    printf("Supervisor started. PID: %d\n\n", getpid());

    // Create workers
    for (int i = 0; i < NUM_WORKERS; i++) {

        workers[i].id = i + 1;
        workers[i].active = 0;

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

        workers[i].pid = pid;
        workers[i].active = 1;

        printf("Worker %d created - PID: %d\n",
               workers[i].id,
               workers[i].pid);
    }

    // Display current worker status
    printf("\nCurrent Worker Status:\n");

    for (int i = 0; i < NUM_WORKERS; i++) {

        printf("Worker %d | PID: %d | Status: %s\n",
               workers[i].id,
               workers[i].pid,
               workers[i].active ? "RUNNING" : "STOPPED");
    }

    printf("\nSupervisor is monitoring the workers...\n\n");

    // Wait for any worker to terminate
    int status;

    pid_t terminated_pid = waitpid(-1, &status, 0);

    if (terminated_pid > 0) {

        // Find which worker terminated
        for (int i = 0; i < NUM_WORKERS; i++) {

            if (workers[i].pid == terminated_pid) {

                workers[i].active = 0;

                printf("\nWorker %d with PID %d terminated.\n",
                       workers[i].id,
                       workers[i].pid);

                printf("Failure detected.\n");
            }
        }
    }

    return 0;
}