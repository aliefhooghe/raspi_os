#ifndef SATAN_HARDWARE_SD_HOST_H_
#define SATAN_HARDWARE_SD_HOST_H_

#include <stdint.h>

//
// raspbery pi zero SD HOST interface driver
// 
void sdhost_init(void);
int sdhost_read_block(uint32_t block_index, uint8_t block[512]);

#endif
