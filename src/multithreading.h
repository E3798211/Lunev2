
#ifndef   MULTITHREADING_H_INCLUDED
#define   MULTITHREADING_H_INCLUDED

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

#ifdef DEBUG
    #define DBG
#else
    #define DBG if(0)
#endif

#define f(x) ((x)*(x))
#define LEFT_BOUND   -25.0
#define RIGHT_BOUND   25.0
#define INTEGRAL_STEP 0.00000001

struct arg_t
{
    double left;
    double right;
    double sum;
    double fill[5];
};

void* routine(void* arg);
int start_threads(struct sysconfig_t* sys, size_t n_threads, 
                         pthread_t tids[], struct arg_t args[]);
int join_threads (struct sysconfig_t* sys, size_t n_threads, 
                         pthread_t tids[], struct arg_t args[], double* sum);

#endif // MULTITHREADING_H_INCLUDED

