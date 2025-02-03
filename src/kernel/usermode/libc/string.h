#ifndef SATAN_USERMODE_LIBC_STRING_H_
#define SATAN_USERMODE_LIBC_STRING_H_

#include <stddef.h>

size_t strlen(const char *);
int strcmp(const char *s1, const char *s2);
int strncmp(const char s1, const char s2, size_t n);

#endif
