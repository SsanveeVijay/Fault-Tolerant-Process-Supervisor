#include <stdio.h>
#include <unistd.h>

int main()
{

    printf("Worker started. PID: %d\n", getpid());

    while (1)
    {
        printf("Worker %d is running...\n", getpid());
        sleep(2);
    }

    return 0;
}