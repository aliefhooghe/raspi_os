
#include "hardware/watchdog.h"
#include "hardware/io_registers.h"
#include "hardware/mmio.h"

#define PM_PASSWORD              0x5A000000u
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020u

void watchdog_init(uint32_t timeout)
{
    mmio_write(REG__PM_WDOG, PM_PASSWORD | timeout);
    mmio_write(REG__PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
}
