#include <stddef.h>
#include <stdint.h>

// GPÏO REGISTER
#define GPFSEL1     0x20200004  // GPIO Function Select Register (for GPIO14 and GPIO15)
#define GPSET0      0x2020001C  // GPIO Pin Output Set Register
#define GPCLR0      0x20200028  // GPIO Pin Output Clear Register
#define GPPUD       0x20200094  // GPIO Pull-up/down Register
#define GPPUDCLK0   0x20200098  // GPIO Pull-up/down Clock Register

// Registers for the mini UART (AUX_MU)
#define AUX_ENABLES     0x20215004  // Enables auxiliary peripherals (SPI1, SPI2, mini UART)
#define AUX_MU_IO_REG   0x20215040  // Data register (read/write for mini UART)
#define AUX_MU_IER_REG  0x20215044  // Interrupt enable register
#define AUX_MU_IIR_REG  0x20215048  // Interrupt identify register
#define AUX_MU_LCR_REG  0x2021504C  // Line control register (data format settings)
#define AUX_MU_MCR_REG  0x20215050  // Modem control register (not used here)
#define AUX_MU_LSR_REG  0x20215054  // Line status register (transmit/receive state)
#define AUX_MU_MSR_REG  0x20215058  // Modem status register (not used here)
#define AUX_MU_SCRATCH  0x2021505C  // Scratch register (general-purpose storage, unused)
#define AUX_MU_CNTL_REG 0x20215060  // Mini UART control register (enables TX/RX)
#define AUX_MU_STAT_REG 0x20215064  // Mini UART status register
#define AUX_MU_BAUD_REG 0x20215068  // Baud rate register


#define UART_LOADER_MAGIC   0x42u
#define KERNEL_START_ADRESS 0x8000u
#define KERNEL_MAX_SIZE     0x8000u

// loader.S symbols
extern void __mmio_write(uint32_t addr, uint32_t value);
extern uint32_t __mmio_read(uint32_t addr);
extern void __cpu_delay(uint32_t cycle_count);
extern void __boot_kernel(void);

static void mini_uart_init(void)
{
    unsigned int ra;

    // GPIO14  TXD0 and TXD1
    // GPIO15  RXD0 and RXD1
    // ALT function 5 for UART1
    // ALT function 0 for UART0

    // Baud rate calculation: ((250,000,000 / 115200) / 8) - 1 = 270

    // Enable the mini UART
    __mmio_write(AUX_ENABLES, 1);

    // Disable interrupts for the mini UART
    __mmio_write(AUX_MU_IER_REG, 0);

    // Disable the transmitter and receiver
    __mmio_write(AUX_MU_CNTL_REG, 0);

    // Configure the mini UART to 8-bit mode
    __mmio_write(AUX_MU_LCR_REG, 3);

    // Clear the FIFO queues and set interrupt modes
    __mmio_write(AUX_MU_IIR_REG, 0xC6);

    // Set baud rate to 115200
    __mmio_write(AUX_MU_BAUD_REG, 270);

    // Configure GPIO14 as TXD and GPIO15 as RXD (ALT5 function)
    ra = __mmio_read(GPFSEL1);
    ra &= ~((7 << 12) | (7 << 15));  // Clear bits 12-14 (GPIO14) and 15-17 (GPIO15)
    ra |= (2 << 12) | (2 << 15);     // Set ALT5 function for GPIO14 (TXD) and GPIO15 (RXD)
    __mmio_write(GPFSEL1, ra);

    // Optional: Disable pull-up/down resistors for GPIO14 and GPIO15
    __mmio_write(GPPUD, 0);
    __cpu_delay(150);
    __mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    __cpu_delay(150);
    __mmio_write(GPPUDCLK0, 0);

    // Enable the transmitter (TX) and receiver (RX)
    __mmio_write(AUX_MU_CNTL_REG, 3);
}


static void __attribute__((section(".text.loader"))) mini_uart_putc(unsigned char c)
{
    // Wait until the transmitter is ready
    while(1)
    {
        if(__mmio_read(AUX_MU_LSR_REG) & 0x20) break;
    }

    // Write the character to the data register
    __mmio_write(AUX_MU_IO_REG, c);
}

static uint8_t __attribute__((section(".text.loader"))) mini_uart_getc(void)
{
    // Wait until data is available to read
    while (1)
    {
        if ((__mmio_read(AUX_MU_LSR_REG) & 0x01) != 0)
            break; // Bit 0: Data ready
    }

    // Read the character from the data register
    return (unsigned char)(__mmio_read(AUX_MU_IO_REG) & 0xFF);
}

static uint32_t __attribute__((section(".text.loader"))) mini_uart_recv(uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        data[i] = mini_uart_getc();
    }
    return size;
}

static uint32_t __attribute__((section(".text.loader"))) serial_load(uint8_t *buffer, uint32_t max_size)
{
    // receive and check magic
    const uint8_t magic = mini_uart_getc();
    if (magic != UART_LOADER_MAGIC)
    {
        return 0;
    }

    // receive size
    uint32_t datasize;
    mini_uart_recv((uint8_t*)&datasize, sizeof(uint32_t));
    if (datasize > max_size) {
        return 0;
    }

    // receive buffer and send byte acks
    for (uint32_t i = 0u; i < datasize; i++) {
        const uint8_t byte = mini_uart_getc();
        mini_uart_putc(byte);
        buffer[i] = byte;
    }

    // send final ack
    mini_uart_putc(UART_LOADER_MAGIC);

    return datasize;
}


// arguments for AArch32
void __attribute__((section(".text.loader"))) loader_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    // initialize mini UART
    mini_uart_init();

    // receive the kernel from serial
    const uint32_t kernel_size = serial_load((uint8_t*)KERNEL_START_ADRESS, KERNEL_MAX_SIZE);
    if (kernel_size == 0)
    {
        // hang if kernel was not received
        while (1);
    }

    // pass to the kernel
    __boot_kernel();
}
