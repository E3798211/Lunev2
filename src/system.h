
#ifndef   SYSTEM_H_INCLUDED
#define   SYSTEM_H_INCLUDED

#include <sched.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "service.h"

#ifdef DEBUG
    #define DBG
#else
    #define DBG if(0)
#endif

#define CACHE_LINE      "/sys/bus/cpu/devices/cpu0/cache/index0/coherency_line_size"

#define CPU_FILENAME_MAX_LEN 100    // Assuming pathname of next files this long
#define CPU_FILE_ONLINE     "/sys/bus/cpu/devices/cpu%d/online"
#define CPU_FILE_PACKAGE    "/sys/bus/cpu/devices/cpu%d/topology/physical_package_id"
#define CPU_FILE_PHYS_ID    "/sys/bus/cpu/devices/cpu%d/topology/core_id"

struct cpu_t 
{
    int number;
    int online;
    int package_id;
    int core_id;
    int location; // bitwised core_id and package_id
};

struct sysconfig_t
{
    size_t cache_line;
    int    n_proc_conf;
    int    n_proc_onln;
    struct cpu_t* cpus;
};

int  get_topology(struct cpu_t* cpus, int n_proc_conf);
/*
    Allocates memory for cpus arg. Must be freed before exit (only when
    function returns EXIT_SUCCESS).
 */
int get_system_config(struct sysconfig_t* const sys);
void delete_config(struct sysconfig_t* sys);

void sort_cpus(struct sysconfig_t* const sys, struct cpu_t* cpus);
int is_used(int cores[], size_t n_cores, int test_core);

#endif // SYSTEM_H_INCLUDED

