#ifndef SATAN_MMIO_H_
#define SATAN_MMIO_H_

#include <stdint.h>

extern void mmio_write(uint32_t addr, uint32_t value);
extern uint32_t mmio_read(uint32_t addr);

#endif
