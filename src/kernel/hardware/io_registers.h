#ifndef SATAN_HARDWARE_REGISTER_H_
#define SATAN_HARDWARE_REGISTER_H_

/**
 *  memory mapped registers adresses
 */

// WARNING: this file contains physical addresses


/**
 * IRQ: hardware interupts
 */
#define REG__IRQ_BASE               0x2000B000u  // The base address for the ARM interrupt register
#define REG__IRQ_PEND_BASE          0x2000B200u  // IRQ basic pending
#define REG__IRQ_PEND_1             0x2000B204u  // IRQ pending 1
#define REG__IRQ_PEND_2             0x2000B208u  // IRQ pending 2
#define REG__IRQ_FIQ_CTL            0x2000B20Cu  // FIQ control
#define REG__IRQ_ENABLE_1           0x2000B210u  // Enable IRQs 1
#define REG__IRQ_ENABLE_2           0x2000B214u  // Enable IRQs 2
#define REG__IRQ_ENABLE_BASIC       0x2000B218u  // Enable Basic IRQs
#define REG__IRQ_DISABLE_1          0x2000B21Cu  // Disable IRQs 1
#define REG__IRQ_DISABLE_2          0x2000B220u  // Disable IRQs 2
#define REG__IRQ_DISABLE_BASIC      0x2000B224u  // Disable Basic IRQs

//
// - IRQ_REG_ENABLE_1/2
//      Writing a 1 to a bit will set the corresponding IRQ enable bit.
//      All other IRQ enable bits are unaffected.
// - IRQ_REG_DISABLE_1/2
//      Writing a 1 to a bit will clear the corresponding IRQ enable bit.
//      All other IRQ enable bits are unaffected.
//

// --------------------------------------------------------------------------------

/**
 * PM: power management / watchdog
 */

#define REG__PM_BASE                0x20100000u  //
#define REG__PM_RSTC                0x2010001Cu  //
#define REG__PM_WDOG                0x20100024u  //
#define REG__PM_RSTS                0x20100020u  //


// --------------------------------------------------------------------------------

/**
 * GPIO: registers
 */
#define REG__GPFSEL1                0x20200004u  // GPIO Function Select Register
#define REG__GPFSEL3                0x2020000Cu  //
#define REG__GPFSEL4                0x20200010u  //

#define REG__GPSET0                 0x2020001Cu  // GPIO Pin Output Set Register
#define REG__GPCLR0                 0x20200028u  // GPIO Pin Output Clear Register
#define REG__GPPUD                  0x20200094u  // GPIO Pull-up/down Register
#define REG__GPPUDCLK0              0x20200098u  // GPIO Pull-up/down Clock Register

// --------------------------------------------------------------------------------

/**
 *  AUX_MU: Registers for the mini UART
 */
#define REG__AUX_ENABLES            0x20215004u  // Enables auxiliary peripherals (SPI1, SPI2, mini UART)
#define REG__AUX_MU_IO_REG          0x20215040u  // Data register (read/write for mini UART)
#define REG__AUX_MU_IER_REG         0x20215044u  // Interrupt enable register
#define REG__AUX_MU_IIR_REG         0x20215048u  // Interrupt identify register
#define REG__AUX_MU_LCR_REG         0x2021504Cu  // Line control register (data format settings)
#define REG__AUX_MU_MCR_REG         0x20215050u  // Modem control register (not used here)
#define REG__AUX_MU_LSR_REG         0x20215054u  // Line status register (transmit/receive state)
#define REG__AUX_MU_MSR_REG         0x20215058u  // Modem status register (not used here)
#define REG__AUX_MU_SCRATCH         0x2021505Cu  // Scratch register (general-purpose storage, unused)
#define REG__AUX_MU_CNTL_REG        0x20215060u  // Mini UART control register (enables TX/RX)
#define REG__AUX_MU_STAT_REG        0x20215064u  // Mini UART status register
#define REG__AUX_MU_BAUD_REG        0x20215068u  // Baud rate register

/**
 *  ARM_TIMER: TODO
 */
#define REG__ARM_TIMER_LOD          0x2000B400u
#define REG__ARM_TIMER_VAL          0x2000B404u
#define REG__ARM_TIMER_CTL          0x2000B408u
#define REG__ARM_TIMER_CLI          0x2000B40Cu
#define REG__ARM_TIMER_RIS          0x2000B410u
#define REG__ARM_TIMER_MIS          0x2000B414u
#define REG__ARM_TIMER_RLD          0x2000B418u
#define REG__ARM_TIMER_DIV          0x2000B41Cu
#define REG__ARM_TIMER_CNT          0x2000B420u

#endif
