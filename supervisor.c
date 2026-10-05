#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_WORKERS 3

struct Worker {
    int id;
    pid_t pid;
    int active;
};

void log_event(const char *message, int worker_id, pid_t pid) {

    FILE *log = fopen("supervisor.log", "a");

    if (log == NULL) {
        perror("Could not open log file");
        return;
    }

    fprintf(log, "%s | Worker %d | PID %d\n",
            message,
            worker_id,
            pid);

    fclose(log);
}

int main() {

    struct Worker workers[NUM_WORKERS];

    printf("Supervisor started. PID: %d\n\n", getpid());

    // Create initial workers
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

    printf("\nCurrent Worker Status:\n");

    for (int i = 0; i < NUM_WORKERS; i++) {

        printf("Worker %d | PID: %d | Status: RUNNING\n",
               workers[i].id,
               workers[i].pid);
    }

    printf("\nSupervisor is monitoring the workers...\n\n");

    // Continuously monitor workers
    while (1) {

        int status;

        pid_t terminated_pid = waitpid(-1, &status, 0);

        if (terminated_pid < 0) {
            perror("waitpid failed");
            break;
        }

        // Find the worker that terminated
        int worker_index = -1;

        for (int i = 0; i < NUM_WORKERS; i++) {

            if (workers[i].pid == terminated_pid) {
                worker_index = i;
                break;
            }
        }

        if (worker_index == -1) {

            printf("Unknown worker terminated. PID: %d\n",
                   terminated_pid);

            continue;
        }

        // Mark worker as inactive
        workers[worker_index].active = 0;

        printf("\nWorker %d with PID %d terminated.\n",
               workers[worker_index].id,
               terminated_pid);

        printf("Failure detected.\n");

        // Log the failure
        log_event("TERMINATED",
                  workers[worker_index].id,
                  terminated_pid);

        printf("Failure logged.\n");

        // Create replacement
        printf("Creating replacement for Worker %d...\n",
               workers[worker_index].id);

        pid_t replacement_pid = fork();

        if (replacement_pid < 0) {

            perror("fork failed while creating replacement");
            continue;
        }

        if (replacement_pid == 0) {

            printf("Starting replacement Worker %d...\n",
                   workers[worker_index].id);

            execl("./worker", "worker", NULL);

            perror("exec failed");
            return 1;
        }

        // Update worker information
        workers[worker_index].pid = replacement_pid;
        workers[worker_index].active = 1;

        printf("Replacement Worker %d created - New PID: %d\n",
               workers[worker_index].id,
               replacement_pid);

        printf("Worker %d is now RUNNING again.\n",
               workers[worker_index].id);

        // Log the successful restart
        log_event("RESTARTED",
                  workers[worker_index].id,
                  replacement_pid);

        printf("Restart logged.\n");

        printf("Supervisor continues monitoring...\n\n");
    }

    return 0;
}