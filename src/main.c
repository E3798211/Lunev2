
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
    int location; // bitwised core_id and package_id
};

struct sysconfig_t
{
    size_t cache_line;
    int    n_proc_conf;
    struct cpu_t* cpus;
};

static int  get_topology(struct cpu_t* cpus, int n_proc_conf);

/*
    Allocates memory for cpus arg. Must be freed before exit (only when
    function returns EXIT_SUCCESS).
 */
static int get_system_config(struct sysconfig_t* const sys);
static void delete_config(struct sysconfig_t* sys);
static void sort_cpus(struct sysconfig_t* const sys, struct cpu_t* cpus);
static int is_used(int cores[], size_t n_cores, int test_core);

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
    if (get_system_config(&sys))
    {
        printf("get_system_config() failed\n");
        return EXIT_FAILURE;
    }
    

    for(int i = 0; i < sys.n_proc_conf; i++)
    {
        printf("core: %d: %s\n", sys.cpus[i].core_id, (sys.cpus[i].online? "online":"offline"));
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

    delete_config(&sys);

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

        // Offline CPUs do not have topology/
        if (!cpus[i].online)
        {
            cpus[i].core_id    = -1;
            cpus[i].package_id = -1;
            cpus[i].location = (cpus[i].package_id << (sizeof(int)/2)) | cpus[i].core_id;
            
            DBG printf("CPU%d: core_id    = %d\n",   i, cpus[i].core_id);
            DBG printf("CPU%d: package id = %d\n",   i, cpus[i].package_id); 
            DBG printf("CPU%d: location   = %d\n\n", i, cpus[i].location);
            
            continue;
        }

        if (get_cpu_attribute(CPU_FILE_PHYS_ID,  i, &cpus[i].core_id))
            return EXIT_FAILURE;
        DBG printf("CPU%d: core_id    = %d\n",   i, cpus[i].core_id);

        if (get_cpu_attribute(CPU_FILE_PACKAGE,  i, &cpus[i].package_id))
            return EXIT_FAILURE;
        DBG printf("CPU%d: package id = %d\n", i, cpus[i].package_id);

        cpus[i].location = (cpus[i].package_id << (sizeof(int)/2)) | cpus[i].core_id;
        DBG printf("CPU%d: location   = %d\n\n", i, cpus[i].location);
    }
    return EXIT_SUCCESS;
}

static int get_system_config(struct sysconfig_t* const sys)
{
    assert(sys);

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

    struct cpu_t cpus[sys->n_proc_conf];
    if (get_topology(cpus, sys->n_proc_conf))
    {
        printf("Failed to get topology\n");
        return EXIT_FAILURE;
    }

    sys->cpus = (struct cpu_t*)calloc(sys->n_proc_conf, sizeof(struct cpu_t));
    if (!sys->cpus)
    {
        printf("calloc() failed\n");
        return EXIT_FAILURE;
    }

    sort_cpus(sys, cpus);

    return EXIT_SUCCESS;
}

static void delete_config(struct sysconfig_t* sys)
{
    free(sys->cpus);
}

static void sort_cpus(struct sysconfig_t* const sys, struct cpu_t* cpus)
{ 
    // Storing indexes
    size_t first_used[sys->n_proc_conf];
    size_t duplicates[sys->n_proc_conf];
    size_t offline   [sys->n_proc_conf];
    // Storing locations
    int    cores_used[sys->n_proc_conf];

    size_t n_first_used = 0;
    size_t n_duplicates = 0;
    size_t n_offline    = 0;
    size_t n_cores_used = 0;

    for(size_t i = 0; i < sys->n_proc_conf; i++)
    {
        if (cpus[i].online)
        {
            if (!is_used(cores_used, n_cores_used, cpus[i].location))
            {
                first_used[n_first_used++] = i;
                cores_used[n_cores_used++] = cpus[i].location;
            }
            else
                duplicates[n_duplicates++] = i;
        }
        else
            offline[n_offline++] = i;
    }

    for(size_t i = 0; i < n_first_used; i++)
        sys->cpus[i] = cpus[first_used[i]];
    for(size_t i = 0; i < n_duplicates; i++)
        sys->cpus[n_first_used + i] = cpus[duplicates[i]];
    for(size_t i = 0; i < n_offline;    i++)
        sys->cpus[n_first_used + n_duplicates + i] = cpus[offline[i]];
}

static int is_used(int cores[], size_t n_cores, int test_core)
{
    for(size_t i = 0; i < n_cores; i++)
        if (cores[i] == test_core)  return 1;
    return 0;
}






