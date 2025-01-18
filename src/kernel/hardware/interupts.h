#ifndef SATAN_HARDWARE_INTERUPTS_H_
#define SATAN_HARDWARE_INTERUPTS_H_

// #define INTERUPT_REG_BASE 0x2000B000u  // The base address for the ARM interrupt register
// #define IRQ_PEND_BASE     0x2000B200u  // IRQ basic pending
// #define IRQ_PEND_1        0x2000B204u  // IRQ pending 1
// #define IRQ_PEND_2        0x2000B208u  // IRQ pending 2
// #define IRQ_FIQ_CTL       0x2000B20Cu  // FIQ control
// #define IRQ_ENABLE_1      0x2000B210u  // Enable IRQs 1
// #define IRQ_ENABLE_2      0x2000B214u  // Enable IRQs 2
// #define IRQ_ENABLE_3      0x2000B218u  // Enable Basic IRQs
// #define IRQ_DISABLE_1     0x2000B21Cu  // Disable IRQs 1
// #define IRQ_DISABLE_2     0x2000B220u  // Disable IRQs 2
// #define IRQ_DISABLE_3     0x2000B224u  // Disable Basic IRQs

#include <stdint.h>

void software_interupt_handler(
    uint32_t syscall_num,
    uint32_t program_status,
    uint32_t arg0, uint32_t arg1);

#endif
