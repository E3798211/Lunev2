
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "service.h"

#ifdef DEBUG
    #define DBG
#else
    #define DBG if(0)
#endif

static void* routine(void* arg);

#define CACHE_LINE   "/sys/bus/cpu/devices/cpu0/cache/index0/coherency_line_size"
#define CPU_FILENAME_MAX_LEN 100    // Assuming pathname of next files this long
#define CPU_FILE_ONLINE   "/sys/bus/cpu/devices/cpu%d/online"
#define CPU_FILE_PACKAGE  "/sys/bus/cpu/devices/cpu%d/topology/physical_package_id"
#define CPU_FILE_PHYS_ID  "/sys/bus/cpu/devices/cpu%d/topology/core_id"

#define N 10

struct cpu_t 
{
    int online;
    int package_id;
    int phys_id;
};

int get_topology(struct cpu_t* cpus, int n_proc_conf);

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
    {
        printf("Failed reading '%s'\n", CACHE_LINE);
        return EXIT_FAILURE;
    }
    DBG printf("cache line length:\t%4lu\n", cache_line_len);
    
    errno = 0;
    int n_proc_conf = sysconf(_SC_NPROCESSORS_CONF);
    if (n_proc_conf < 0)
    {
        perror("sysconf(_SC_NPROCESSORS_CONF)");
        return EXIT_FAILURE;
    }
    DBG printf("configured processors:\t%4d\n", n_proc_conf);

    struct cpu_t cpus[n_proc_conf];
    get_topology(cpus, n_proc_conf);
    
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


int get_topology(struct cpu_t* cpus, int n_proc_conf)
{
    assert(cpus);
    assert(n_proc_conf > 0);

    for(int i = 0; i < n_proc_conf; i++)
    {
        char pathname[CPU_FILENAME_MAX_LEN];

        // Online
        snprintf(pathname, CPU_FILENAME_MAX_LEN, CPU_FILE_ONLINE, i);
        pathname[CPU_FILENAME_MAX_LEN - 1] = '\0';
        DBG printf("reading '%s':\n", pathname);

        // CPU0 is always online, this file does not exist
        if (i != 0)
        {
            if (read_number(pathname, "%d", &cpus[i].online))
            {
                printf("Failed reading '%s'\n", pathname);
                return EXIT_FAILURE;
            }
        }
        else
            cpus[i].online = 1;
        DBG printf("CPU%d: online     = %d\n", i, cpus[i].online);

        // Physical ID 
        snprintf(pathname, CPU_FILENAME_MAX_LEN, CPU_FILE_PHYS_ID, i);
        pathname[CPU_FILENAME_MAX_LEN - 1] = '\0';
        DBG printf("reading '%s':\n", pathname);

        if (read_number(pathname, "%d", &cpus[i].phys_id))
        {
            printf("Failed reading '%s'\n", pathname);
            return EXIT_FAILURE;
        }
        DBG printf("CPU%d: phys_id    = %d\n", i, cpus[i].phys_id);


        // Package ID 
        snprintf(pathname, CPU_FILENAME_MAX_LEN, CPU_FILE_PACKAGE, i);
        pathname[CPU_FILENAME_MAX_LEN - 1] = '\0';
        DBG printf("reading '%s':\n", pathname);

        if (read_number(pathname, "%d", &cpus[i].package_id))
        {
            printf("Failed reading '%s'\n", pathname);
            return EXIT_FAILURE;
        }
        DBG printf("CPU%d: package id = %d\n\n", i, cpus[i].package_id);
    }
}










