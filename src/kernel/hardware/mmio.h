#ifndef SATAN_MMIO_H_
#define SATAN_MMIO_H_

#include <stdint.h>

extern void mmio_write(uintptr_t addr, uint32_t value);
extern uint32_t mmio_read(uintptr_t addr);

#endif
