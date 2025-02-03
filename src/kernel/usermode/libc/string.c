
#include "string.h"
#include <stddef.h>

size_t strlen(const char *s)
{
    size_t len = 0u;
    while (*(s++))
        len++;
    return len;
}
