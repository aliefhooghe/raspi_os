
#include <stdint.h>

#include "hardware/gpio.h"
#include "hardware/cpu.h"
#include "hardware/io_registers.h"
#include "hardware/mmio.h"
#include "kernel.h"


static uint32_t _select_gpfsel_register(uint32_t gpio_index, uint32_t *shift)
{
    // Each GPIO_GPFSEL* register handler 10 GPIO wires (3 bits each)
    const uint32_t gpfsel_index = gpio_index / 10u;
    *shift = 3u * (gpio_index % 10u);


    return REG__GPIO_GPFSEL(gpfsel_index);
}

static uint32_t _select_gppud_clk_register(uint32_t pin)
{
    return pin < 32u ? REG__GPIO_GPPUDCLK0 : REG__GPIO_GPPUDCLK1;
}

//
//  Public API
//

void gpio_select_function(uint32_t pin, gpio_pin_func_t function)
{
    if (pin >= GPIO_PIN_COUNT)
        kernel_fatal_error("gpio: invalid pin number");

    uint32_t gpfsel_shift = 0u;
    const uint32_t gpfsel_register = _select_gpfsel_register(pin, &gpfsel_shift);
    uint32_t gpfsel = mmio_read(gpfsel_register);

    // clear function values
    gpfsel &= ~(GPIO_F_MASK << gpfsel_shift);

    // set new function
    gpfsel |= ((function & GPIO_F_MASK) << gpfsel_shift);

    mmio_write(gpfsel_register, gpfsel);
}

void gpio_set_pin_mode(uint32_t pin, gpio_pin_mode_t mode)
{
    if (pin >= GPIO_PIN_COUNT)
        kernel_fatal_error("gpio: invalid pin number");

    const uint32_t gppud_clk = _select_gppud_clk_register(pin);
    const uint32_t bit = pin & 0xfu;  // (pin % 32u)

    // write mode
    mmio_write(REG__GPIO_GPPUD, mode);
    cpu_delay(150);

    // enable clock for selected pin
    mmio_write(gppud_clk, 1u << bit);
    cpu_delay(150);

    // clean up
    mmio_write(REG__GPIO_GPPUD, 0x0u);
    mmio_write(gppud_clk, 0x0u);
}
