
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
    if (access(filename, F_OK))
    {
        perror("access()");
        return EXIT_FAILURE;
    }

    errno = 0;
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("open()");
        return EXIT_FAILURE;
    }
    
    char buffer[BUF_LEN];
    errno = 0;
    int len = read(fd, buffer, BUF_LEN);
    if (len < 0)
    {
        perror("read()");
        return EXIT_FAILURE;
    }

    buffer[len] = '\0';
    sscanf(buffer, fmt, value);

    return EXIT_SUCCESS;
}


