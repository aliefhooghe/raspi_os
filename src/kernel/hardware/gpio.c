
#include <stdint.h>

#include "hardware/gpio.h"
#include "hardware/io_registers.h"
#include "hardware/mmio.h"
#include "kernel.h"


//
//
//
static uint32_t _select_gpfsel_register(uint32_t gpio_index, uint32_t *shift)
{
    // Each GPIO_GPFSEL* register handler 10 GPIO wires (3 bits each)
    const uint32_t gpfsel_index = gpio_index / 10u;
    *shift = 3u * (gpio_index % 10u);

    if (gpfsel_index >= GPIO_GPFSEL_COUNT)
    {
        kernel_fatal_error("gpio: invalid gpio index");
    }

    return REG__GPIO_GPFSEL(gpfsel_index);
}

void gpio_select_function(uint32_t gpio_index, uint8_t function)
{
    uint32_t gpfsel_shift = 0u;
    const uint32_t gpfsel_register = _select_gpfsel_register(gpio_index, &gpfsel_shift);
    uint32_t gpfsel = mmio_read(gpfsel_register);

    // clear function values
    gpfsel &= ~(GPIO_F_MASK << gpfsel_shift);

    // set new function
    gpfsel |= ((function & GPIO_F_MASK) << gpfsel_shift);

    mmio_write(gpfsel_register, gpfsel);
}
