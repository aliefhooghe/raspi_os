#ifndef SATAN_WATCHDOG_H_
#define SATAN_WATCHDOG_H_

#include <stdint.h>

#define PM_BASE 0x20100000
#define PM_RSTC (PM_BASE + 0x1C)
#define PM_WDOG (PM_BASE + 0x24)
#define PM_RSTS (PM_BASE + 0x20)
#define PM_PASSWORD 0x5A000000
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020

void watchdog_init(uint32_t timeout);

#endif
