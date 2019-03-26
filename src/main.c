
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
#define N 10

#define CACHE_LINE      "/sys/bus/cpu/devices/cpu0/cache/index0/coherency_line_size"

#define CPU_FILENAME_MAX_LEN 100    // Assuming pathname of next files this long
#define CPU_FILE_ONLINE     "/sys/bus/cpu/devices/cpu%d/online"
#define CPU_FILE_PACKAGE    "/sys/bus/cpu/devices/cpu%d/topology/physical_package_id"
#define CPU_FILE_PHYS_ID    "/sys/bus/cpu/devices/cpu%d/topology/core_id"

struct cpu_t 
{
    int online;
    int package_id;
    int core_id;
};

struct sysconfig_t
{
    size_t cache_line;
    int    n_proc_conf;
    struct cpu_t* cpus;
};

static int get_topology(struct cpu_t* cpus, int n_proc_conf);

/*
    Allocates memory for cpus arg. Must be freed before exit (only when
    function returns EXIT_SUCCESS).
 */
static int get_system_config(struct sysconfig_t* const sys,
                             struct cpu_t** const cpus);

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

    struct sysconfig_t sys;
    struct cpu_t* cpus;
    if (get_system_config(&sys, &cpus))
    {
        printf("get_system_config() failed\n");
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


static int get_cpu_attribute(const char* const file_template, int core_num, 
                             int* attr)
{
    char pathname[CPU_FILENAME_MAX_LEN];
    
    snprintf(pathname, CPU_FILENAME_MAX_LEN, file_template, core_num);
    pathname[CPU_FILENAME_MAX_LEN - 1] = '\0';
    DBG printf("reading '%s':\n", pathname);

    if (read_value(pathname, "%d", attr)) 
    {
        printf("Failed reading '%s'\n", pathname);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int get_topology(struct cpu_t* cpus, int n_proc_conf)
{
    assert(cpus);
    assert(n_proc_conf > 0);

    for(int i = 0; i < n_proc_conf; i++)
    {
        // CPU0 is always online
        if (i != 0)
        {
            if (get_cpu_attribute(CPU_FILE_ONLINE, i, &cpus[i].online))
                return EXIT_FAILURE;
        }
        else
            cpus[i].online = 1;
        DBG printf("CPU%d: online     = %d\n",   i, cpus[i].online);

        if (get_cpu_attribute(CPU_FILE_PHYS_ID,  i, &cpus[i].core_id))
            return EXIT_FAILURE;
        DBG printf("CPU%d: core_id    = %d\n",   i, cpus[i].core_id);

        if (get_cpu_attribute(CPU_FILE_PACKAGE,  i, &cpus[i].package_id))
            return EXIT_FAILURE;
        DBG printf("CPU%d: package id = %d\n\n", i, cpus[i].package_id);    
    }
    return EXIT_SUCCESS;
}

static int get_system_config(struct sysconfig_t* const sys,
                             struct cpu_t** const cpus)
{
    assert(sys);
    assert(cpus);

    if (read_value(CACHE_LINE, "%lu", &sys->cache_line))
    {
        printf("Failed reading '%s'\n", CACHE_LINE);
        return EXIT_FAILURE;
    }
    DBG printf("cache line length:\t%4lu\n", sys->cache_line);
    
    errno = 0;
    sys->n_proc_conf = sysconf(_SC_NPROCESSORS_CONF);
    if (sys->n_proc_conf < 0)
    {
        perror("sysconf(_SC_NPROCESSORS_CONF)");
        return EXIT_FAILURE;
    }
    DBG printf("configured processors:\t%4d\n\n", sys->n_proc_conf);

    *cpus = (struct cpu_t*)calloc(sys->n_proc_conf, sizeof(struct cpu_t));
    if (!*cpus)
    {
        printf("calloc() failed\n");
        return EXIT_FAILURE;
    }

    if (get_topology(*cpus, sys->n_proc_conf))
    {
        printf("Failed to get topology\n");
        free(*cpus);
        return EXIT_FAILURE;
    }



    return EXIT_SUCCESS;
}








