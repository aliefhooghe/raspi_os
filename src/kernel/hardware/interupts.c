
#include <stdint.h>

#include "hardware/io_registers.h"
#include "hardware/irq.h"
#include "hardware/mmio.h"
#include "hardware/system_timer.h"

void irq_handler(void)
{
    const uint32_t grp1_pending = mmio_read(REG__IRQ_PEND_1);

    if (grp1_pending & IRQ1_SYSTEM_TIMER_3)
    {
        mmio_write(REG__SYS_TIMER_CS, SYS_TIMER_CS_M3);
        const uint32_t delay = 10000;  // 10 ms at 1Mhz
        const uint32_t next_tick = mmio_read(REG__SYS_TIMER_CLO) + delay;
        mmio_write(REG__SYS_TIMER_C3, next_tick);
    }
}
