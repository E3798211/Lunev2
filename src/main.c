
#define _GNU_SOURCE

#include <sched.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "service.h"
#include "system.h"
#include "multithreading.h"

#ifdef DEBUG
    #define DBG
#else
    #define DBG if(0)
#endif

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <n_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    size_t n_threads = 0;
    if (get_positive(argv[1], &n_threads))
    {
        printf("Invalid input\n");
        return EXIT_FAILURE;
    }
    DBG printf("Requested for threads\t%4lu\n\n", n_threads);

    struct sysconfig_t sys;
    if (get_system_config(&sys))
    {
        printf("get_system_config() failed\n");
        return EXIT_FAILURE;
    }
   
    size_t n_thread_args = 
        ((n_threads < sys.n_proc_onln)? sys.n_proc_onln : n_threads);
    DBG printf("Creating  args:\t\t%4lu\n", n_thread_args);

    errno = 0;
    struct arg_t* thread_args = 
        aligned_alloc(sys.cache_line, sys.cache_line * n_thread_args);
    if (!thread_args)
    {
        perror("posix_memalign()");
        return EXIT_FAILURE;
    }
    DBG printf("Allocated %lu bytes each %lu bytes aligned to %p\n\n",
               sys.cache_line * n_thread_args, sizeof(struct arg_t), 
               thread_args);
    
    pthread_t tids[((n_threads < sys.n_proc_onln)? 
                     sys.n_proc_onln : n_threads)];

    DBG printf("Starting threads\n");
    if (start_threads(&sys, n_threads, tids, thread_args))
    {
        printf("start_threads() failed\n");
        return EXIT_FAILURE;
    }
    
    DBG printf("Joining threads\n");
    double sum = 0;
    if (join_threads(&sys, n_threads, tids, thread_args, &sum))
    {
        printf("join_threads() failed\n");
        return EXIT_FAILURE;
    }

    printf("%lg\n", sum);


    delete_config(&sys);
    free(thread_args);

    return 0;
}





