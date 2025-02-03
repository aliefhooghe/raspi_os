#ifndef SATAN_USERMODE_LIBC_STDIO_H_
#define SATAN_USERMODE_LIBC_STDIO_H_

#include <stdarg.h>
#include <stddef.h>

int putchar(int c);
int puts(const char *s);

int printf(const char *restrict format, ...);
int vprintf(const char *restrict format, va_list ap);

int getchar(void);
char *gets_s(char*, size_t size);

#endif
