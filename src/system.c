
#include "system.h"

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

int get_topology(struct cpu_t* cpus, int n_proc_conf)
{
    assert(cpus);
    assert(n_proc_conf > 0);

    for(int i = 0; i < n_proc_conf; i++)
    {
        cpus[i].number = i;
        DBG printf("CPU%d: number     = %d\n",   i, cpus[i].number);

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
            cpus[i].location = (cpus[i].package_id << (sizeof(int)/2)) | 
                                cpus[i].core_id;
            DBG printf("\n");
            continue;
        }

        if (get_cpu_attribute(CPU_FILE_PHYS_ID,  i, &cpus[i].core_id))
            return EXIT_FAILURE;
        DBG printf("CPU%d: core_id    = %d\n",   i, cpus[i].core_id);

        if (get_cpu_attribute(CPU_FILE_PACKAGE,  i, &cpus[i].package_id))
            return EXIT_FAILURE;
        DBG printf("CPU%d: package id = %d\n", i, cpus[i].package_id);

        cpus[i].location = (cpus[i].package_id << (sizeof(int)/2)) | 
                            cpus[i].core_id;
        DBG printf("CPU%d: location   = %d\n\n", i, cpus[i].location);
    }
    return EXIT_SUCCESS;
}

int get_system_config(struct sysconfig_t* const sys)
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
    DBG printf("configured processors:\t%4d\n", sys->n_proc_conf);

    errno = 0;
    sys->n_proc_onln = sysconf(_SC_NPROCESSORS_ONLN);
    if (sys->n_proc_onln < 0)
    {
        perror("sysconf(_SC_NPROCESSORS_ONLN)");
        return EXIT_FAILURE;
    }
    DBG printf("online processors:\t%4d\n\n", sys->n_proc_onln);

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

void delete_config(struct sysconfig_t* sys)
{
    free(sys->cpus);
}

void sort_cpus(struct sysconfig_t* const sys, struct cpu_t* cpus)
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

int is_used(int cores[], size_t n_cores, int test_core)
{
    for(size_t i = 0; i < n_cores; i++)
        if (cores[i] == test_core)  return 1;
    return 0;
}

