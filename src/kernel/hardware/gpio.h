#ifndef SATAN_GPIO_H_
#define SATAN_GPIO_H_

#include <stdint.h>

//
// 3 bit value for GPIO selected function
typedef enum {
    GPIO_F_INPUT            = 0x0u, // 0b000
    GPIO_F_OUTPUT           = 0x1u, // 0b001
    GPIO_F_ALT_0            = 0x4u, // 0b100
    GPIO_F_ALT_1            = 0x5u, // 0b101
    GPIO_F_ALT_2            = 0x6u, // 0b110
    GPIO_F_ALT_3            = 0x7u, // 0b111
    GPIO_F_ALT_4            = 0x3u, // 0b011
    GPIO_F_ALT_5            = 0x2u, // 0b010
} gpio_pin_func_t;

#define GPIO_F_MASK           0x7u

//
// GPIO mode configuration
typedef enum {
    GPIO_MODE_PULL_FLOATING  = 0x0u,
    GPIO_MODE_PULL_DOWN      = 0x1u,
    GPIO_MODE_PULL_UP        = 0x2u,
} gpio_pin_mode_t;

//
// valid pin number is from 0 to 53 included
#define GPIO_PIN_COUNT           54u

//
// Select gpio function. pin from 0 to 53
void gpio_select_function(uint32_t pin, gpio_pin_func_t function);

//
// Select gpio pin mode
void gpio_set_pin_mode(uint32_t pin, gpio_pin_mode_t mode);

#endif
