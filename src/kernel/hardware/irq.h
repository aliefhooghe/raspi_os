#ifndef SATAN_HARDWARE_IRQ_H_
#define SATAN_HARDWARE_IRQ_H_

//
// see REG_IRQ_ENABLE/DISABLE/PENDING_BASIC
#define IRQ_BASIC_ARM_TIMER        0x00000001u
#define IRQ_BASIC_ARM_MAILBOX      0x00000002u
#define IRQ_BASIC_ARM_DOORBELL0    0x00000004u
#define IRQ_BASIC_ARM_DOORBELL1    0x00000008u
#define IRQ_BASIC_ARM_GPU0_HALTED  0x00000010u
#define IRQ_BASIC_ARM_GPU1_HALTED  0x00000020u

//
// REG_IRQ_ENABLE/DISABLE/PENDING_BASIC 1
#define IRQ1_SYSTEM_TIMER_0 0x00000001u // System timer 0  (used by VideoCore GPU)
#define IRQ1_SYSTEM_TIMER_1 0x00000002u // System timer 1  (free)
#define IRQ1_SYSTEM_TIMER_2 0x00000004u // System timer 2  (used by VideoCore GPU)
#define IRQ1_SYSTEM_TIMER_3 0x00000008u // System timer 3  (free). Used for userspace premption
#define IRQ1_AUX_INT        0x20000000u // Aux uart interupts


// REG_IRQ_ENABLE/DISABLE/PENDING_BASIC 2
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

#endif
