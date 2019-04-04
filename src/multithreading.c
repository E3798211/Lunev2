
#include "multithreading.h"

void* routine(void* arg)
{
    struct arg_t* bounds = (struct arg_t*)arg;
    double left  = bounds->left;
    double right = bounds->right;
    double sum = 0;
    for(double i = left; i < right; i += INTEGRAL_STEP)
        sum += INTEGRAL_STEP*f(i);

    bounds->sum = sum;
    return NULL;
}

int start_threads(struct sysconfig_t* sys, size_t n_threads, 
                        pthread_t tids[], struct arg_t args[])
{
    const double step = (RIGHT_BOUND - LEFT_BOUND) / n_threads;
    
    // Normal
    if (n_threads <= sys->n_proc_onln)
    {
        cpu_set_t cpuset;
        pthread_attr_t attr;
        errno = 0;
        if (pthread_attr_init(&attr))
        {
            perror("pthread_attr_init()");
            return EXIT_FAILURE;
        }
         
        for(size_t i = 0; i < sys->n_proc_onln; i++)
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
            DBG printf("Created %lu'th thread on %d'th proc\n",
                        i, sys->cpus[i].number);
        }
        pthread_attr_destroy(&attr);
        return EXIT_SUCCESS;
    }

    // else
    for(size_t i = 0; i < n_threads; i++)
    {
        args[i].left  = LEFT_BOUND + step * i;
        args[i].right = LEFT_BOUND + step * (i + 1);
        
        errno = 0;
        DBG printf("%p\n", &args[i]);
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
    size_t i = 0;
    for(i = 0; i < n_threads; i++)
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
    for(; i < sys->n_proc_onln; i++)
    {
        errno = 0;
        DBG printf("Joining %lu'th thread\n", i);
        if (pthread_join(tids[i], NULL))
        {
            perror("pthread_join()");
            printf("Failed to join %lu'th thread\n", i);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

