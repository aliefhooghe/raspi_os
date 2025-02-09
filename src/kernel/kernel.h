#ifndef SATAN_KERNEL_H_
#define SATAN_KERNEL_H_

#include <stdint.h>

// kernel entry point. Called from reset handler
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags);
void kernel_fatal_error(const char *reason);



#endif
