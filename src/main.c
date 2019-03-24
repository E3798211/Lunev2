
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "service.h"

#ifdef DEBUG
    #define DBG
#else
    #define DBG if(0)
#endif

static void* routine(void* arg);

#define CACHE_LINE "/sys/bus/cpu/devices/cpu0/cache/index0/coherency_line_size"
#define N 10

struct proc_t 
{
    int online;
    int package_id;
    int phys_id;
};

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

    size_t cache_line_len;
    if (read_number(CACHE_LINE, "%lu", &cache_line_len))
        return EXIT_FAILURE;
    DBG printf("cache line length:\t%4lu\n", cache_line_len);
    
    errno = 0;
    int n_proc_conf = sysconf(_SC_NPROCESSORS_CONF);
    if (n_proc_conf < 0)
    {
        perror("sysconf(_SC_NPROCESSORS_CONF)");
        return EXIT_FAILURE;
    }
    DBG printf("configured processors:\t%4d\n", n_proc_conf);

    
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





