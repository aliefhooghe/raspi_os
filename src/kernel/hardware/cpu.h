#ifndef SATAN_CPU_H_
#define SATAN_CPU_H_

#include <stdint.h>

extern void cpu_delay(uint32_t cycle_count);

//
//  # APSR – Application Program Status Register
//
//  # CPSR – Current Program Status Register
//
//   +========+========+=========================================+
//   | Bit(s) | name   | info                                    |
//   +========+========+=========================================+
//   | 31     | N      |                                         |
//   | 30     | Z      |                                         |
//   | 29     | C      |                                         |
//   | 28     | V      |                                         |
//   | 27-8   | unused |                                         |
//   | 7      | I      | Disables IRQ interrupts when it is set. |
//   | 6      | F      | Disables FIQ interrupts when it is set. |
//   | 5      | T      | Thumb mode                              |
//   | 4-0    | M      | cpu mode (see bellow)                   |
//   +--------+--------+-----------------------------------------+
//
//   CPSR_C alias: bit 7-0 (CPSR Control)
//
//  # SPSR – Saved Program Status Register
//
//
// 
// Notes:
// - The presence of particular processor modes and states depends on whether
//   the processor implements the relevant architecture extension
// 
//            Mode               Encoding       Function                                                                     SecurityState   PrivilegeLevel
#define CPU_CPSR_MODE_MASK        0X1Fu
#define CPU_CPSR_MODE_USER        0X10u  // Unprivileged mode in which mostapplications run                           Both            PL0
#define CPU_CPSR_MODE_FIQ         0X11u  // Entered on an FIQ interrupt exception                                     Both            PL1
#define CPU_CPSR_MODE_IRQ         0X12u  // Entered on an IRQ interrupt exception                                     Both            PL1
#define CPU_CPSR_MODE_SUPERVISOR  0X13u  // Entered on reset or when a Supervisor Call instruction (SVC) is executed  Both            PL1
#define CPU_CPSR_MODE_MONITOR     0X16u  // Implemented with Security Extensions.                                     Secure only     PL1
#define CPU_CPSR_MODE_ABORT       0X17u  // Entered on a memory access exception                                      Both            PL1
#define CPU_CPSR_MODE_HYP         0X1Au  // Implemented with Virtualization Extensions.                               Non-secure      PL2
#define CPU_CPSR_MODE_UNDEF       0X1Bu  // Entered when an undefined instruction executed                            Both            PL1
#define CPU_CPSR_MODE_SYSTEM      0X1Fu  // Privileged mode, sharing the register view with User mode                 Both            PL1

#define CPU_CPSR_DISABLE_FIQ      (1 << 6)
#define CPU_CPSR_DISABLE_IRQ      (1 << 7)

// a formatter: registre acceccibles par mode.
// 10000 	User 	PC, R14 to R0, CPSR
// 10001 	FIQ 	PC, R14_fiq to R8_fiq, R7 to R0, CPSR, SPSR_fiq
// 10010 	IRQ 	PC, R14_irq, R13_irq, R12 to R0, CPSR, SPSR_irq
// 10011 	Supervisor 	PC, R14_svc, R13_svc, R12 to R0, CPSR, SPSR_svc
// 10111 	Abort 	PC, R14_abt, R13_abt, R12 to R0, CPSR, SPSR_abt
// 11011 	Undefined 	PC, R14_und, R13_und, R12 to R0, CPSR, SPSR_und
// 11111 	System 	PC, R14 to R0, CPSR

//
// Enable irq globally
void cpu_irq_enable(void);

//
// Disable irq globally
void cpu_irq_disable(void);

//
// get cpu execution mode
uint8_t cpu_get_execution_mode(void);

//
// wait for interupt
void cpu_wait_for_interupt(void);

#endif
