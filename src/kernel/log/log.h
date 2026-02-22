#ifndef SATAN_KERNEL_LOGS_H_
#define SATAN_KERNEL_LOGS_H_

// kernel log interface
#define KERNEL_ENABLE_LOG

void kernel_puts(const char* str);

#ifdef KERNEL_ENABLE_LOG
void kernel_log(const char *restrict format, ...);
#else
#define kernel_log(...) (void)(0, __VA_ARGS__)
#endif

#endif
