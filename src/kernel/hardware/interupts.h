#ifndef SATAN_HARDWARE_INTERUPTS_H_
#define SATAN_HARDWARE_INTERUPTS_H_

#include <stdint.h>

// harware interupt request
#define IRQ_REG_BASE          0x2000B000u  // The base address for the ARM interrupt register
#define IRQ_REG_PEND_BASE     0x2000B200u  // IRQ basic pending
#define IRQ_REG_PEND_1        0x2000B204u  // IRQ pending 1
#define IRQ_REG_PEND_2        0x2000B208u  // IRQ pending 2
#define IRQ_REG_FIQ_CTL       0x2000B20Cu  // FIQ control
#define IRQ_REG_ENABLE_1      0x2000B210u  // Enable IRQs 1
#define IRQ_REG_ENABLE_2      0x2000B214u  // Enable IRQs 2
#define IRQ_REG_ENABLE_BASIC  0x2000B218u  // Enable Basic IRQs
#define IRQ_REG_DISABLE_1     0x2000B21Cu  // Disable IRQs 1
#define IRQ_REG_DISABLE_2     0x2000B220u  // Disable IRQs 2
#define IRQ_REG_DISABLE_BASIC 0x2000B224u  // Disable Basic IRQs

/**
 * - IRQ_REG_ENABLE_1/2
 *      Writing a 1 to a bit will set the corresponding IRQ enable bit.
 *      All other IRQ enable bits are unaffected.
 * - IRQ_REG_DISABLE_1/2
 *      Writing a 1 to a bit will clear the corresponding IRQ enable bit.
 *      All other IRQ enable bits are unaffected.
 */

// mask apply on ENABLE/DISABLE 1
#define IRQ1_AUX_INT         0x20000000u // 1 << 29

// mask apply on ENABLE/DISABLE 2
#define IRQ2_I2C_SPI_SLV_INT 0x00000800u // 1 << 11
#define IRQ2_PWA0            0x00002000u // 1 << 13
#define IRQ2_PWA1            0x00004000u // 1 << 14
#define IRQ2_SMI             0x00010000u // 1 << 16
#define IRQ2_GPIO_INT_0      0x00020000u // 1 << 17
#define IRQ2_GPIO_INT_1      0x00040000u // 1 << 18
#define IRQ2_GPIO_INT_2      0x00080000u // 1 << 19
#define IRQ2_GPIO_INT_3      0x00100000u // 1 << 20
#define IRQ2_I2C_INT         0x00200000u // 1 << 21
#define IRQ2_SPI_INT         0x00400000u // 1 << 22
#define IRQ2_PCM_INT         0x00800000u // 1 << 23
#define IRQ2_UART_INT        0x02000000u // 1 << 25


uint32_t syscall(
    uint32_t arg0,
    uint32_t arg1
);

#endif
