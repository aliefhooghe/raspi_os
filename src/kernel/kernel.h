#ifndef SATAN_KERNEL_H_
#define SATAN_KERNEL_H_

#include <stdint.h>

//
// kernel entry point. Called from reset handler
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags);

//
// configure the mmu to use the kernel translation table.
// called from the svc handler.
void kernel_restore_translation_table(void);

//
// fatal error handlers
void kernel_fatal_error(const char *reason);
void kernel_unhandled_interupt_fatal_error(void);


#endif
