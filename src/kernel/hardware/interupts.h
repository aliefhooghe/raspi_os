#ifndef SATAN_HARDWARE_INTERUPTS_H_
#define SATAN_HARDWARE_INTERUPTS_H_

#include <stdint.h>

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
