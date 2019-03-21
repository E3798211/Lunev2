
#ifndef   SERVICE_H_INCLUDED
#define   SERVICE_H_INCLUDED

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_LEN 100

int get_positive(char const* str, size_t* val);
int read_number(const char* const filename, const char* const fmt,
                void* const value);

#endif // SERVICE_H_INCLUDED

