
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

static int get_positive(char const* str, size_t* val)
{
    char* endptr = NULL;
    errno = 0;
    long long value = strtol(str, &endptr, 10);
    if(errno == ERANGE || *endptr|| value < 1)
        return EXIT_FAILURE;
    *val = value;
    return EXIT_SUCCESS;
}

static void* routine(void* arg)
{
    *((int*)arg) = 5;
    return arg;
}


#define N 10

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <n_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    size_t n_threads;
    if (get_positive(argv[1], &n_threads))
    {
        printf("Invalid input\n");
        return EXIT_FAILURE;
    }






/*
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
*/

    return 0;
}

