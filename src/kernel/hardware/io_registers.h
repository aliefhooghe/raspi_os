#ifndef SATAN_HARDWARE_REGISTER_H_
#define SATAN_HARDWARE_REGISTER_H_

// Note: The arm peripheral reference manual gives us bus adresses.
//   mapping bus to physical addresses: 0x7Exxxxxx  => 0x20xxxxxx
// 

//
// MMIO range is mapped for the kernel only. MMIOs are not mapped
//   in the processes virtual memory space
// 
// mmio range: 0x20000000 - 0x20FFFFFF
#define IO_REG_START                0x20000000u
#define IO_REG_END                  0x21000000u


// 
//  memory mapped IO registers adresses (physical addresses)
// 


// --------------------------------------------------------------------------------
// 
// IRQ: hardware interupts
// 
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
// 
// PM: power management / watchdog
// 
#define REG__PM_BASE                0x20100000u  // TODO: doc is hard to find for this WatchDog.
#define REG__PM_RSTC                0x2010001Cu  //
#define REG__PM_WDOG                0x20100024u  //
#define REG__PM_RSTS                0x20100020u  //


// --------------------------------------------------------------------------------
// 
// GPIO: General Purpose Input Output registers
// 
#define REG__GPIO_GPFSEL(n)         (0x20200000u + 4u * n)  // GPIO Function Select Registers (n in [0, 5])
#define REG__GPIO_GPSET0            0x2020001Cu  // GPIO Pin Output Set Register
#define REG__GPIO_GPCLR0            0x20200028u  // GPIO Pin Output Clear Register
#define REG__GPIO_GPPUD             0x20200094u  // GPIO Pull-up/down Register
#define REG__GPIO_GPPUDCLK0         0x20200098u  // GPIO Pull-up/down Clock 0 Register
#define REG__GPIO_GPPUDCLK1         0x2020009Cu  // GPIO Pull-up/down Clock 1 Register

// --------------------------------------------------------------------------------
// 
//  AUX_MU: Registers for the mini UART
// 
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

// --------------------------------------------------------------------------------
// 
//  ARM_TIMER: TODO
// 
#define REG__ARM_TIMER_LOD          0x2000B400u
#define REG__ARM_TIMER_VAL          0x2000B404u
#define REG__ARM_TIMER_CTL          0x2000B408u
#define REG__ARM_TIMER_CLI          0x2000B40Cu
#define REG__ARM_TIMER_RIS          0x2000B410u
#define REG__ARM_TIMER_MIS          0x2000B414u
#define REG__ARM_TIMER_RLD          0x2000B418u
#define REG__ARM_TIMER_DIV          0x2000B41Cu
#define REG__ARM_TIMER_CNT          0x2000B420u


// --------------------------------------------------------------------------------
// 
//  SDCARD: SD Host registers
//
//  Note: the EMMC module registrers can only be accessed as 32 bit register:
//  i.e the two Leasst Significant Bits of the addresss are always zero
//
#define REG__SDCARD_ARG2            0x20300000u  // ACMD23 Argument
#define REG__SDCARD_BLKSIZECNT      0x20300004u  // Block size and count
#define REG__SDCARD_ARG1            0x20300008u  // Argument
#define REG__SDCARD_CMDTM           0x2030000Cu  // Command and Transfer Mode
#define REG__SDCARD_RESP0           0x20300010u  // Response buts 031:000
#define REG__SDCARD_RESP1           0x20300014u  // Response buts 063:032
#define REG__SDCARD_RESP2           0x20300018u  // Response buts 095:064
#define REG__SDCARD_RESP3           0x2030001Cu  // Response buts 127:096
#define REG__SDCARD_DATA            0x20300020u  // Data
#define REG__SDCARD_HOST_STATUS     0x20300024u  // Host Status: intended for debug, not polling
#define REG__SDCARD_CONTROL0        0x20300028u  // Host Configuration bits
#define REG__SDCARD_CONTROL1        0x2030002Cu  // Host Configuration bits
#define REG__SDCARD_INTERRUPT       0x20300030u  // Interrupt Flags
#define REG__SDCARD_IRPT_MASK       0x20300034u  // Interrupt Flag Enable
#define REG__SDCARD_IRPT_EN         0x20300038u  // Interrupt Generation Enable
#define REG__SDCARD_CONTROL2        0x2030003Cu  // Host Configuration bits
#define REG__SDCARD_FORCE_IRPT      0x20300050u  // Force Interrupt Event
#define REG__SDCARD_BOOT_TIMEOUT    0x20300070u  // Timeout in boot mode
#define REG__SDCARD_DBG_SEL         0x20300074u  // Debug Bus Configuration
#define REG__SDCARD_EXRDFIFO_EN     0x20300084u  // Extension FIFO Enable
#define REG__SDCARD_TUNE_STEP       0x20300088u  // Delay per card clock tuning step
#define REG__SDCARD_TUNE_STEP_STD   0x2030008Cu  // Card clock tuning steps for SDR
#define REG__SDCARD_TUNE_STEPS_DDR  0x20300090u  // Card clock tuning steps for DDR
#define REG__SDCARD_SPI_INT_SPT     0x203000F0u  // SPI Interrupt Support
#define REG__SDCARD_SLOT_ISR_VER    0x203000FCu  // Slot Interrupt Status and Version

#endif
