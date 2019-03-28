
#include "multithreading.h"

void* routine(void* arg)
{
    struct arg_t* bounds = (struct arg_t*)arg;
    for(double x = bounds->left; x < bounds->right; x += INTEGRAL_STEP)
        bounds->sum += INTEGRAL_STEP*f(x);
    return bounds;
}

int start_threads(struct sysconfig_t* sys, size_t n_threads, 
                        pthread_t tids[], struct arg_t args[])
{
    const double step = (RIGHT_BOUND - LEFT_BOUND) / n_threads;

    // Second thread does nothing
    if (n_threads == 1)
    {
        for(size_t i = 0; i < 2; i++)
        {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(sys->cpus[i].number, &cpuset);
    
            args[i].left  = LEFT_BOUND;
            args[i].right = RIGHT_BOUND;
    
            errno = 0;
            if (pthread_create(&tids[i], NULL, routine, &args[i]))
            {
                perror("pthread_create()");
                printf("Failed to create %lu'th thread\n", i);
                return EXIT_FAILURE;
            }
    
            errno = 0;
            if (pthread_setaffinity_np(tids[i], sizeof(cpu_set_t), &cpuset))
            {
                perror("pthread_setaffinity_np()");
                printf("Failed to move %lu'th thread to %d'th core\n", i, 
                       sys->cpus[i].number);
                return EXIT_FAILURE;
            }
            DBG printf("Thread %lu set to CPU%d\n", i, sys->cpus[i].number);
        }
        return EXIT_SUCCESS;
    }

    // Normal
    if (n_threads <= sys->n_proc_onln)
    {
        cpu_set_t cpuset;
        pthread_attr_t attr;
        if (pthread_attr_init(&attr))
        {
            perror("pthread_attr_init()");
            return EXIT_FAILURE;
        }
        
        for(size_t i = 0; i < n_threads; i++)
        {
            CPU_ZERO(&cpuset);
            CPU_SET(sys->cpus[i].number, &cpuset);
            
            args[i].left  = LEFT_BOUND + step * i;
            args[i].right = LEFT_BOUND + step * (i + 1);
 
            errno = 0;
            if (pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset))
            {
                perror("pthread_attr_setaffinity_np()");
                return EXIT_FAILURE;
            }

            errno = 0;
            if (pthread_create(&tids[i], &attr, routine, &args[i]))
            {
                perror("pthread_create()");
                printf("Failed to create %lu'th thread\n", i);
                return EXIT_FAILURE;
            }

/* 
            errno = 0;
            if (pthread_setaffinity_np(tids[i], sizeof(cpu_set_t), &cpuset))
            {
                perror("pthread_setaffinity_np()");
                printf("Failed to move %lu'th thread to %d'th core\n", i, 
                       sys->cpus[i].number);
                return EXIT_FAILURE;
            }
            DBG printf("Thread %lu set to CPU%d\n", i, sys->cpus[i].number);
*/
        }
        pthread_attr_destroy(&attr);
        return EXIT_SUCCESS;
    }

    // else
    
    DBG printf("Sched is asked for help\n");
    for(size_t i = 0; i < n_threads; i++)
    {
        args[i].left  = LEFT_BOUND + step * i;
        args[i].right = LEFT_BOUND + step * (i + 1);

        errno = 0;
        if (pthread_create(&tids[i], NULL, routine, &args[i]))
        {
            perror("pthread_create()");
            printf("Failed to create %lu'th thread\n", i);
            return EXIT_FAILURE;
        } 
    }

    return EXIT_SUCCESS;
}

int join_threads (struct sysconfig_t* sys, size_t n_threads, 
                         pthread_t tids[], struct arg_t args[], double* sum)
{
    *sum = 0;
    for(size_t i = 0; i < n_threads; i++)
    {
        errno = 0;
        DBG printf("Joining %lu'th thread\n", i);
        if (pthread_join(tids[i], NULL))
        {
            perror("pthread_join()");
            printf("Failed to join %lu'th thread\n", i);
            return EXIT_FAILURE;
        }
        *sum += args[i].sum;
    }

    return EXIT_SUCCESS;
}

