
#include "hardware/watchdog.h"

void watchdog_init(uint32_t timeout)
{
    *(volatile unsigned int *)PM_WDOG = PM_PASSWORD | timeout;
    *(volatile unsigned int *)PM_RSTC = PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET;
}
