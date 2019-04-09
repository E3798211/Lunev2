
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

/*
int start_threads(struct sysconfig_t* sys, size_t n_threads, 
                        pthread_t tids[], struct arg_t args[])
{
    const double step = (RIGHT_BOUND - LEFT_BOUND) / n_threads;
    
    // Setting first thread to the 0-th core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(sys->cpus[0].number, &cpuset);
        
    errno = 0;
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    {
        perror("pthread_setaffinity_np()");
        return EXIT_FAILURE;
    }

    // Normal
//    if (n_threads <= sys->n_proc_onln)
    if (n_threads < sys->n_proc_onln)
    {
        pthread_attr_t attr;
        errno = 0;
        if (pthread_attr_init(&attr))
        {
            perror("pthread_attr_init()");
            return EXIT_FAILURE;
        }
        
        // All other live on different cpus
*
        for(size_t i = 1; i < sys->n_proc_onln; i++)
        {
            CPU_ZERO(&cpuset);
            CPU_SET(sys->cpus[i].number, &cpuset);    // CPU0 for main thread
    
            args[i].left  = LEFT_BOUND + step * (i - 1);
            args[i].right = LEFT_BOUND + step * i;

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
*

// New try
        for(size_t i = 0; i < sys->n_proc_onln - 1; i++)
        {
            CPU_ZERO(&cpuset);
            CPU_SET(sys->cpus[i + 1].number, &cpuset);    // CPU0 for main thread
    
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
    for(size_t i = 1; i < n_threads + 1; i++)
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
*/

int start_threads(struct sysconfig_t* sys, size_t n_threads, 
                        pthread_t tids[], struct arg_t args[])
{
    const double step = (RIGHT_BOUND - LEFT_BOUND) / n_threads;
    
    // Setting first thread to the 0-th core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(sys->cpus[0].number, &cpuset);
        
    errno = 0;
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    {
        perror("pthread_setaffinity_np()");
        return EXIT_FAILURE;
    }

    // Normal
//    if (n_threads <= sys->n_proc_onln)
    pthread_attr_t attr;
    errno = 0;
    if (pthread_attr_init(&attr))
    {
        perror("pthread_attr_init()");
        return EXIT_FAILURE;
    }
    
    size_t n_real_threads = 
        ((n_threads < sys->n_proc_onln)? sys->n_proc_onln - 1 : n_threads);

    for(size_t i = 0; i < n_real_threads; i++)
    {
        CPU_ZERO(&cpuset);
        CPU_SET(sys->cpus[(i + 1) % sys->n_proc_onln].number, &cpuset);    // CPU0 for main thread

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
                    i, sys->cpus[(i + 1) % sys->n_proc_onln].number);
    }

    return EXIT_SUCCESS;
}
/*
int join_threads (struct sysconfig_t* sys, size_t n_threads, 
                         pthread_t tids[], struct arg_t args[], double* sum)
{
    *sum = 0; 
    size_t i = 1;
    for(i = 1; i < n_threads + 1; i++)
    {
        DBG printf("Joining %lu'th thread\n", i);
        errno = 0;
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
        DBG printf("Joining %lu'th thread\n", i);
        errno = 0;
        if (pthread_join(tids[i], NULL))
        {
            perror("pthread_join()");
            printf("Failed to join %lu'th thread\n", i);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
*/


// Last version
/*
int join_threads (struct sysconfig_t* sys, size_t n_threads, 
                         pthread_t tids[], struct arg_t args[], double* sum)
{
    *sum = 0; 
    size_t i = 0;
    for(i = 0; i < n_threads; i++)
    {
        DBG printf("Joining %lu'th thread\n", i);
        errno = 0;
        if (pthread_join(tids[i], NULL))
        {
            perror("pthread_join()");
            printf("Failed to join %lu'th thread\n", i);
            return EXIT_FAILURE;
        }
        *sum += args[i].sum;
    }
    for(; i < sys->n_proc_onln - 1; i++)
    {
        DBG printf("Joining %lu'th thread\n", i);
        errno = 0;
        if (pthread_join(tids[i], NULL))
        {
            perror("pthread_join()");
            printf("Failed to join %lu'th thread\n", i);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
*/


int join_threads (struct sysconfig_t* sys, size_t n_threads, 
                         pthread_t tids[], struct arg_t args[], double* sum)
{
    *sum = 0; 
    size_t n_real_threads = 
        ((n_threads < sys->n_proc_onln)? sys->n_proc_onln - 1 : n_threads);

for(size_t i = 0; i < n_real_threads; i++)
    {
        DBG printf("Joining %lu'th thread\n", i);
        errno = 0;
        if (pthread_join(tids[i], NULL))
        {
            perror("pthread_join()");
            printf("Failed to join %lu'th thread\n", i);
            return EXIT_FAILURE;
        }
        
        if (i < n_threads)
            *sum += args[i].sum;
    }

    return EXIT_SUCCESS;
}




