#ifndef SATAN_USERMODE_LIBC_STDIO_H_
#define SATAN_USERMODE_LIBC_STDIO_H_

#include <stdarg.h>

int putchar(int c);
int puts(const char *s);

int printf(const char *restrict format, ...);
int vprintf(const char *restrict format, va_list ap);

int getchar(void);

#endif
