#ifndef SATAN_CPU_H_
#define SATAN_CPU_H_

#include <stdint.h>

extern void cpu_delay(uint32_t cycle_count);

/**
 * Notes:
 * - The presence of particular processor modes and states depends on whether the processor implements the relevant architecture extension
 */

//      Mode                Encoding       Function                                                                  SecurityState   PrivilegeLevel
#define CPU_MODE_USER        0X10u      // Unprivileged mode in which most applications run                          Both            PL0
#define CPU_MODE_FIQ         0X11u      // Entered on an FIQ interrupt exception                                     Both            PL1
#define CPU_MODE_IRQ         0X12u      // Entered on an IRQ interrupt exception                                     Both            PL1
#define CPU_MODE_SUPERVISOR  0X13u      // Entered on reset or when a Supervisor Call instruction (SVC) is executed  Both            PL1
#define CPU_MODE_MONITOR     0X16u      // Implemented with Security Extensions.                                     Secure only     PL1
#define CPU_MODE_ABORT       0X17u      // Entered on a memory access exception                                      Both            PL1
#define CPU_MODE_HYP         0X1Au      // Implemented with Virtualization Extensions.                               Non-secure      PL2
#define CPU_MODE_UNDEF       0X1Bu      // Entered when an undefined instruction executed                            Both            PL1
#define CPU_MODE_SYSTEM      0X1Fu      // Privileged mode, sharing the register view with User mode                 Both            PL1

uint8_t cpu_get_execution_mode(void);

/**
 *  APSR – Application Program Status Register
 *  CPSR – Current Program Status Register
 *  SPSR – Saved Program Status Register
 */



#endif
