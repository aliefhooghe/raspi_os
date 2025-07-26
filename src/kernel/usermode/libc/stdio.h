#ifndef SATAN_USERMODE_LIBC_STDIO_H_
#define SATAN_USERMODE_LIBC_STDIO_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define WRITE_BUFFER_SIZE 128u

typedef struct {
    int fd;
    size_t write_buffer_cursor;
    uint8_t write_buffer[WRITE_BUFFER_SIZE];
} FILE;

// stub the libc initialization
#define DECLARE_STDOUT FILE stdoutfd; FILE *stdout = &stdoutfd; get_stdout(stdout)
void get_stdout(FILE *);


// files manipulations
int fflush(FILE *stream);
size_t fread(void *restrict ptr, size_t size, size_t n, FILE *restrict stream);
size_t fwrite(const void *restrict ptr, size_t size, size_t n, FILE *restrict stream);

// STDIO
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
