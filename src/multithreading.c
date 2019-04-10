
#include "multithreading.h"

void* routine(void* arg)
{
    register struct arg_t* bounds = (struct arg_t*)arg;
    register double left  = bounds->left;
    register double right = bounds->right;
    register double sum = 0;
    for(register double i = left; i < right; i += INTEGRAL_STEP)
        sum += INTEGRAL_STEP*f(i);

    bounds->sum = sum;

    pthread_exit(NULL);

    return NULL;
}

static void init_thread_args(struct arg_t args[], size_t n_args, double step)
{
    for(size_t i = 0; i < n_args; i++)
    {
        args[i].left  = LEFT_BOUND + step * i;
        args[i].right = LEFT_BOUND + step * (i + 1);
    }
}

static int set_thread_affinity(pthread_attr_t* attr, int cpu_num)
{
    cpu_set_t cpuset;

    errno = 0;
    if (pthread_attr_init(attr))
    {
        perror("pthread_attr_init");
        return EXIT_FAILURE;
    }

    CPU_ZERO(&cpuset);
    CPU_SET(cpu_num, &cpuset);
    
    errno = 0;
    if (pthread_attr_setaffinity_np(attr, sizeof(cpuset), &cpuset))
    {
        perror("pthread_attr_setaffinity_np");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int start_idle_threads(struct sysconfig_t* sys, size_t n_threads, 
                        pthread_t tids[], struct arg_t args[])
{ 
    const double step = (RIGHT_BOUND - LEFT_BOUND) / n_threads;
    
    init_thread_args(args, sys->n_proc_onln, step);

    pthread_attr_t attrs[sys->n_proc_onln];
    for(size_t i = 0; i < sys->n_proc_onln; i++)
        if (set_thread_affinity(&attrs[i], sys->cpus[i].number))
        {
            printf("set_thread_affinity() failed\n");
            return EXIT_FAILURE;
        }

    for(size_t i = 1; i < sys->n_proc_onln; i++)
    {
        errno = 0;
        if (pthread_create(&tids[i], &attrs[i], routine, &args[i]))
        {
            perror("pthread_create()");
            return EXIT_FAILURE;
        }
    }
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(sys->cpus[0].number, &cpuset);
 
    errno = 0;
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset))
    {
        perror("pthread_attr_setaffinity_np");
        return EXIT_FAILURE;
    }

    tids[0] = pthread_self();
    routine(&args[0]);

    return EXIT_SUCCESS;
}

static int start_useful_threads(struct sysconfig_t* sys, size_t n_threads, 
                        pthread_t tids[], struct arg_t args[])
{
    const double step = (RIGHT_BOUND - LEFT_BOUND) / n_threads;

    const int n_threads_per_cpu = n_threads / sys->n_proc_onln;
    const int extra_threads     = n_threads % sys->n_proc_onln;

    return EXIT_SUCCESS;
}

int start_threads(struct sysconfig_t* sys, size_t n_threads, 
                        pthread_t tids[], struct arg_t args[])
{
    const double step = (RIGHT_BOUND - LEFT_BOUND) / n_threads;
/*
    if (n_threads < sys->n_proc_onln)
    {
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
    
        pthread_attr_t attr;
        errno = 0;
        if (pthread_attr_init(&attr))
        {
            perror("pthread_attr_init()");
            return EXIT_FAILURE;
        }

        for(size_t i = 0; i < sys->n_proc_onln - 1; i++)
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
    }
    else
    {
        for(size_t i = 0; i < n_threads; i++)
        {
            DBG printf("Starting %lu'th thread\n", i);
            
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
        DBG printf("Scheduler invoked\n");
    }
*/
/*
    cpu_set_t cpuset;
    pthread_attr_t attr;
    errno = 0;
    if (pthread_attr_init(&attr))
    {
        perror("pthread_attr_init()");
        return EXIT_FAILURE;
    }

    size_t n_real_threads = 
        ((n_threads < sys->n_proc_onln)? sys->n_proc_onln - 1 : n_threads - 1);

    for(size_t i = 0; i < n_real_threads + 1; i++)
    {
        args[i].left  = LEFT_BOUND + step * i;
        args[i].right = LEFT_BOUND + step * (i + 1);
    }

    // Placing threads
    const size_t threads_per_cpu = n_real_threads/sys->n_proc_onln;
    size_t       extra_threads   = n_real_threads%sys->n_proc_onln;

    int cpus = sys->n_proc_onln;
    int threads_left[cpus];
    for(size_t i = 0; i < cpus; i++)
        threads_left[i] = threads_per_cpu;
    while(extra_threads--)
    {
        threads_left[cpus - 1]++;
        cpus--;
    }

    DBG
    {
        for(int i = 0; i < sys->n_proc_onln; i++)
            printf("CPU%d -> %d\n", i, threads_left[i]);
    }

    // size_t current_cpu = sys->n_proc_onln - 1;
    size_t current_cpu = 0;
    for(size_t i = 0; i < n_real_threads; i++)
    {
        CPU_ZERO(&cpuset);
        
        if (threads_left[current_cpu])      threads_left[current_cpu]--;
        else
        {
            current_cpu++;
            threads_left[current_cpu]--;
        }
        CPU_SET(sys->cpus[current_cpu].number, &cpuset);

        // CPU_SET(sys->cpus[(sys->n_proc_onln - i - 1) % sys->n_proc_onln].number, &cpuset);    // CPU0 for main thread

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
                    i, sys->cpus[current_cpu].number);
    }

    // Setting first thread to the 0-th core
    CPU_ZERO(&cpuset);
    CPU_SET(sys->cpus[0].number, &cpuset);
        
    errno = 0;
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    {
        perror("pthread_setaffinity_np()");
        return EXIT_FAILURE;
    }
    routine(&args[n_real_threads]);
*/

    size_t n_real_threads = 
        ((n_threads < sys->n_proc_onln)? sys->n_proc_onln - 1 : n_threads - 1);


    return start_idle_threads(sys, n_threads, tids, args);

    int n_threads_per_cpu = n_real_threads/sys->n_proc_onln;
    int extra_threads     = n_real_threads%sys->n_proc_onln;

    int load[sys->n_proc_onln];
    for(size_t i = 0; i < sys->n_proc_onln; i++)
        load[i] = n_threads_per_cpu;
    for(size_t i = 0; i < extra_threads; i++)
        load[sys->n_proc_onln - i - 1]++;

    cpu_set_t cpuset;
    pthread_attr_t attrs[sys->n_proc_onln];
    for(size_t i = 0; i < sys->n_proc_onln; i++)
    {
        errno = 0;
        if (pthread_attr_init(&attrs[i]))
        {
            perror("pthread_attr_init()");
            return EXIT_FAILURE;
        }
    }

    for(size_t i = 0; i < n_real_threads + 1; i++)
    {
        args[i].left  = LEFT_BOUND + step *  i;
        args[i].right = LEFT_BOUND + step * (i + 1);
    }

    size_t current_cpu = sys->n_proc_onln - 1;
    for (size_t i = 0; i < n_real_threads; i++)
    {
        CPU_ZERO(&cpuset);
        
        if (load[current_cpu] > 0)  
            load[current_cpu]--;
        else
        {
            current_cpu--;
            load[current_cpu]--;
        }
        
        CPU_SET(sys->cpus[current_cpu].number, &cpuset);



        
        printf("%d\n", CPU_COUNT(&cpuset));





        errno = 0;
        if (pthread_attr_setaffinity_np(&attrs[i], sizeof(cpu_set_t), &cpuset))
        {
            perror("pthread_attr_setaffinity_np()");
            return EXIT_FAILURE;
        }

        errno = 0;
        if (pthread_create(&tids[i], &attrs[i], routine, &args[i]))
        {
            perror("pthread_create()");
            printf("Failed to create %lu'th thread\n", i);
            return EXIT_FAILURE;
        }
        DBG printf("Created %lu'th thread on %d'th proc\n",
                    i, sys->cpus[current_cpu].number);
    }

    CPU_ZERO(&cpuset);
    CPU_SET(sys->cpus[0].number, &cpuset);
        
    errno = 0;
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
    {
        perror("pthread_setaffinity_np()");
        return EXIT_FAILURE;
    }
    
    DBG printf("Creating main thread on %d'th proc\n",
                sys->cpus[0].number);
    routine(&args[n_real_threads]);

    for(size_t i = 0; i < sys->n_proc_onln; i++)
        pthread_attr_destroy(&attrs[i]);

    return EXIT_SUCCESS;
}



int join_threads (struct sysconfig_t* sys, size_t n_threads, 
                         pthread_t tids[], struct arg_t args[], double* sum)
{
    *sum = 0;
/*
    if (n_threads < sys->n_proc_onln)
    {
        for(size_t i = 0; i < sys->n_proc_onln - 1; i++)
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
    }
    else
    {
        for(size_t i = 0; i < n_threads; i++)
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
    }
*/

    size_t n_real_threads = 
        ((n_threads < sys->n_proc_onln)? sys->n_proc_onln - 1 : n_threads - 1);

//    for(size_t i = 0; i < n_real_threads; i++)
    for(size_t i = 0; i < n_threads; i++)
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
    *sum += args[0].sum;

    return EXIT_SUCCESS;
}




