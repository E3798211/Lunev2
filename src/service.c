
#include "service.h"

int get_positive(char const* str, size_t* val)
{
    char* endptr = NULL;
    errno = 0;
    long long value = strtol(str, &endptr, 10);
    if(errno == ERANGE || *endptr|| value < 1)
        return EXIT_FAILURE;
    *val = value;
    return EXIT_SUCCESS;
}

int read_number(const char* const filename, const char* const fmt,
                void* const value)
{
    errno = 0;
    FILE* fin = fopen(filename, "r");
    if (!fin)
    {
        perror("fopen()");
        return EXIT_FAILURE;
    }

    errno = 0;
    fscanf(fin, fmt, value);
    int status = errno;
    
    fclose(fin);
    return ((status)? EXIT_FAILURE : EXIT_SUCCESS);
}








