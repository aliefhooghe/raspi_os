#ifndef SATAN_USERMODE_LIBC_STDIO_H_
#define SATAN_USERMODE_LIBC_STDIO_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef struct FILE FILE;

extern FILE *stdout;

// files manipulations
FILE *fopen(const char *restrict path, const char *restrict mode);
FILE *fdopen(int fd, const char *mode);
int fclose(FILE* file);

int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);

size_t fread(void *restrict ptr, size_t size, size_t n, FILE *restrict stream);
size_t fwrite(const void *restrict ptr, size_t size, size_t n, FILE *restrict stream);
int fflush(FILE *stream);

// int putchar(int c);

int fputs(const char *restrict s, FILE *restrict stream);
// int puts(const char *s);

// int snprintf(char *restrict str, size_t size, const char *restrict format, ...);
// int vsnprintf(char *restrict str, size_t size, const char *restrict format, va_list ap);

int fprintf(FILE *restrict stream, const char *restrict format, ...);
int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap);

int getchar(void);
char *gets_s(char*, size_t size);

#endif
