#include <stddef.h>
#include <stdint.h>

#define GPFSEL1 0x20200004  // GPIO Function Select Register (for GPIO14 and GPIO15)
#define GPSET0  0x2020001C  // GPIO Pin Output Set Register
#define GPCLR0  0x20200028  // GPIO Pin Output Clear Register
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

// GPIO14  TXD0 and TXD1
// GPIO15  RXD0 and RXD1
// ALT function 5 for UART1
// ALT function 0 for UART0

// Baud rate calculation: ((250,000,000 / 115200) / 8) - 1 = 270

static const char *welcome_message =
"  ________        __       ___________        __       _____  ___           ______      ________\r\n"
" /\"       )      /\"\"\\     (\"     _   \")      /\"\"\\     (\\\"   \\|\"  \\         /    \" \\    /\"\r\n"
"(:   \\___/      /    \\     )__/  \\\\__/      /    \\    |.\\\\   \\    |       // ____  \\  (:   \\___/\r\n"
" \\___  \\       /' /\\  \\       \\\\_ /        /' /\\  \\   |: \\.   \\\\  |      /  /    ) :)  \\___  \\\r\n"
"  __/  \\\\     //  __'  \\      |.  |       //  __'  \\  |.  \\    \\. |     (: (____/ //    __/  \\\\\r\n"
" /\" \\   :)   /   /  \\\\  \\     \\:  |      /   /  \\\\  \\ |    \\    \\ |      \\        /    /\" \\   :)\r\n"
"(_______/   (___/    \\___)     \\__|     (___/    \\___) \\___|\\____\\)       \\\"_____/    (_______/\r\n"
;

static inline void mmio_write(uint32_t addr, uint32_t value)
{
    *(volatile uint32_t*)addr = value;
}

static inline uint32_t mmio_read(uint32_t addr)
{
    return *((volatile uint32_t*)addr);
}

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1\n"
           " bne __delay_%=\n"
         : "=r"(count): [count]"0"(count) : "cc");
}

static void uart_init()
{
    unsigned int ra;

    // Enable the mini UART
    mmio_write(AUX_ENABLES, 1);

    // Disable interrupts for the mini UART
    mmio_write(AUX_MU_IER_REG, 0);

    // Disable the transmitter and receiver
    mmio_write(AUX_MU_CNTL_REG, 0);

    // Configure the mini UART to 8-bit mode
    mmio_write(AUX_MU_LCR_REG, 3);

    // Clear the FIFO queues and set interrupt modes
    mmio_write(AUX_MU_IIR_REG, 0xC6);

    // Set baud rate to 115200
    mmio_write(AUX_MU_BAUD_REG, 270);

    // Configure GPIO14 as TXD and GPIO15 as RXD (ALT5 function)
    ra = mmio_read(GPFSEL1);
    ra &= ~((7 << 12) | (7 << 15));  // Clear bits 12-14 (GPIO14) and 15-17 (GPIO15)
    ra |= (2 << 12) | (2 << 15);     // Set ALT5 function for GPIO14 (TXD) and GPIO15 (RXD)
    mmio_write(GPFSEL1, ra);

    // Optional: Disable pull-up/down resistors for GPIO14 and GPIO15
    mmio_write(GPPUD, 0);
    delay(150);
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    mmio_write(GPPUDCLK0, 0);

    // Enable the transmitter (TX) and receiver (RX)
    mmio_write(AUX_MU_CNTL_REG, 3);
}

void uart_putc(unsigned char c)
{
    // Wait until the transmitter is ready
    while(1)
    {
        if(mmio_read(AUX_MU_LSR_REG) & 0x20) break;
    }

    // Write the character to the data register
    mmio_write(AUX_MU_IO_REG, c);
}

unsigned char uart_getc(void)
{
    // Wait until data is available to read
    while (1)
    {
        if ((mmio_read(AUX_MU_LSR_REG) & 0x01) != 0)
            break; // Bit 0: Data ready
    }

    // Read the character from the data register
    return (unsigned char)(mmio_read(AUX_MU_IO_REG) & 0xFF);
}


void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((unsigned char)str[i]);
}

void uart_put_int(uint32_t x)
{
    char result[12] = "";
    int index = 11;

    while (x > 0) {
        result[--index] = '0' + (x % 10);
        x /= 10;
    }

    uart_puts(result + index);
}

// arguments for AArch32
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    // initialize UART for Raspi0
    uart_init();

    // print a welcome message ;)
    uart_puts(welcome_message);

    char car = '\r';
    do {
        if (car == '\r')
        {
            uart_puts("\r\nsatan ~ ");
        }
        else {
            uart_putc(car);
        }
        car = uart_getc();
    } while (1);
}
