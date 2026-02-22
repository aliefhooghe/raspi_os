
#include "system_timer.h"

#include "hardware/io_registers.h"
#include "hardware/irq.h"
#include "hardware/mmio.h"
#include <stdint.h>

void system_timer_init(void)
{
    const uint32_t delay = 20000;  // 20 ms at 1Mhz
    const uint32_t next_tick = mmio_read(REG__SYS_TIMER_CLO) + delay;
    mmio_write(REG__SYS_TIMER_C3, next_tick);


    // Enable arm timer irqs
    mmio_write(REG__IRQ_ENABLE_1, IRQ1_SYSTEM_TIMER_3);
}
