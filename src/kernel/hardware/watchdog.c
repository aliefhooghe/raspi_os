
#include "hardware/watchdog.h"
#include "hardware/mmio.h"

void watchdog_init(uint32_t timeout)
{
    mmio_write(PM_WDOG, PM_PASSWORD | timeout);
    mmio_write(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
}
