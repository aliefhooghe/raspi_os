
#include "string.h"
#include <stddef.h>

size_t strlen(const char *s)
{
    size_t len = 0u;
    while (*(s++))
        len++;
    return len;
}

int strcmp(const char *s1, const char *s2)
{

}

int strncmp(const char s1, const char s2, size_t n)
{

}
