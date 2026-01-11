
#include <stdint.h>

#include "hardware/watchdog.h"
#include "hardware/io_registers.h"
#include "hardware/mmio.h"

#define PM_PASSWORD              0x5A000000u
#define PM_WDOG_TIME_MASK        0x000FFFFF
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020u
#define PM_RSTC_WRCFG_CLR        0xFFFFFFCFu


void watchdog_reboot(void)
{
    const uint16_t timeout_ticks = 1u;
    mmio_write(REG__PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET );
    mmio_write(REG__PM_WDOG, PM_PASSWORD | (timeout_ticks & PM_WDOG_TIME_MASK));

    // avoid UBA
    for (;;);
}
