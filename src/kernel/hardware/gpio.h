#ifndef SATAN_GPIO_H_
#define SATAN_GPIO_H_

#include <stdint.h>

//
// 3 bit value for GPIO selected function
#define GPIO_F_INPUT    0x0u // 0b000
#define GPIO_F_OUTPUT   0x1u // 0b001
#define GPIO_F_ALT_0    0x4u // 0b100
#define GPIO_F_ALT_1    0x5u // 0b101
#define GPIO_F_ALT_2    0x6u // 0b110
#define GPIO_F_ALT_3    0x7u // 0b111
#define GPIO_F_ALT_4    0x3u // 0b011
#define GPIO_F_ALT_5    0x2u // 0b010
#define GPIO_F_MASK     0x7u

//
// Select gpio function.
void gpio_select_function(uint32_t gpio_index, uint8_t function);

#endif
