
#include <stdio.h>
#include <pthread.h>

static
void* routine(void* arg)
{
    *((int*)arg) = 5;
    return arg;
}

#define N 10

int main()
{
    pthread_t tids[N];
    int buf[N];

    for(size_t i = 0; i < N; i++)
    {
        if (pthread_create(&tids[i], NULL, routine, &buf[i]))
            printf("Failed to create thread\n");
    }

    for(size_t i = 0; i < N; i++)
    {
        if (pthread_join(tids[i], NULL))
            printf("Failed to join thread\n");
    }

    for(size_t i = 0; i < N; i++)
        printf("%4lu = %4d\n", i, buf[i]);

    return 0;
}

