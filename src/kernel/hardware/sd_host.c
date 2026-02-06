
#include <stdint.h>

#include "sd_host.h"
#include "hardware/cpu.h"
#include "hardware/gpio.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmio.h"
#include "kernel.h"


// Status register: REG__SDCARD_STATUS
//
#define SDCARD_STATUS_DAT_LEVEL1_MASK          0x1E000000u  // value of data lines DAT7 to DAT4
#define SDCARD_STATUS_CMD_LEVEL_MASK           0x01000000u  // value of command line CMD
#define SDCARD_STATUS_DAT_LEVEL0_MASK          0x00F00000u  // value of data lines DAT3 to DAT0
#define SDCARD_STATUS_READ_TRANSFER_MASK       0x00000200u  // new data can be read from EMMC
#define SDCARD_STATUS_WRITE_TRANSFER_MASK      0x00000100u  // new data can be written to EMMC
#define SDCARD_STATUS_DAT_ACTIVE               0x00000004u  // at least one data line is active
#define SDCARD_STATUS_DAT_INHIBIT              0x00000002u  // data lines still used by previous data transfer
#define SDCARD_STATUS_CMD_INHIBIT              0x00000001u  // command line still used by previous command


// Interupt register: REG__SDCARD_INTERUPT
#define SDCARD_INTERUPT_ACMD_ERR               0x01000000u  // auto command erro
#define SDCARD_INTERUPT_DEND_ERR               0x00400000u  // end bit on data line was not 1
#define SDCARD_INTERUPT_DCRC_ERR               0x00200000u  // data crc error
#define SDCARD_INTERUPT_DTO_ERR                0x00100000u  // timeout on data line
#define SDCARD_INTERUPT_CBAD_ERR               0x00080000u  // incorrect command index in response
#define SDCARD_INTERUPT_CEND_ERR               0x00040000u  // end bit on command line was not 1
#define SDCARD_INTERUPT_CCRC_ERR               0x00020000u  // command crc error
#define SDCARD_INTERUPT_CTO_ERR                0x00010000u  // timeout on command line
#define SDCARD_INTERUPT_ERR                    0x00008000u  // an error has occured
#define SDCARD_INTERUPT_ENDBOOT                0x00004000u  // boot operation has terminated
#define SDCARD_INTERUPT_BOOTACK                0x00002000u  // boot acknowledge has been received
#define SDCARD_INTERUPT_RETUNE                 0x00001000u  // clock retune request was made
#define SDCARD_INTERUPT_CARD                   0x00000100u  // card made interrupt request
#define SDCARD_INTERUPT_READ_READY             0x00000020u  // DATA register contains DATA to be read
#define SDCARD_INTERUPT_WRITE_READY            0x00000010u  // DATA can be writtent to DATA register
#define SDCARD_INTERUPT_BLOCK_GAP              0x00000004u  // data transfert has stopped at block gap
#define SDCARD_INTERUPT_DATA_DONE              0x00000002u  // data transfert has finished
#define SDCARD_INTERUPT_CMD_DONE               0x00000001u  // command has finished

// Control1 register: REG__SDCARD_CONTROL1
// note:
// CLK_STABLE seems contrary to its name only to indicate that there was a rising edge
// on the clk_emmc input but not that the frequency of this clock is actually stable.
#define SDCARD_CONTROL1_SRST_DATA              0x04000000u  // Reset the data handling circuit
#define SDCARD_CONTROL1_SRST_CMD               0x02000000u  // Reset the command handling circuit
#define SDCARD_CONTROL1_SRST_HC                0x01000000u  // Reset the complete host circuit
#define SDCARD_CONTROL1_DATA_TOUNIT            0x000F0000u  // Data timeout unit exponent (1111 = disabled, x = TMCLK * 2^(x + 13))
#define SDCARD_CONTROL1_CLK_FREQ8              0x0000FF00u  // SD clock base divider LSBs
#define SDCARD_CONTROL1_CLK_FREQ_MS2           0x000000C0u  // SD clock base divider MSBs
#define SDCARD_CONTROL1_GENSEL                 0x00000020u  // Mode of clock generation: 0=divided, 1=programmable
#define SDCARD_CONTROL1_CLK_EN                 0x00000004u  // SD clock enable
#define SDCARD_CONTROL1_STABLE                 0x00000002u  //
#define SDCARD_CONTROL1_INTLEN                 0x00000001u  // Clock enable for internal EMMC clocks for power saving

// supported standards:
//  - SD Host Controller Standard Specification Version 3.0 Draft 1.0: base controller spec
//  - SDIO card specification version 3.0: ignored, for exemples wifi cards, etc...
//  - SD Memory Card Specification Draft version 3.0: how to talk with an SD memory card
//  - SD Memory Card Security Specification version 1.01
//  - MMC Specification version 3.31,4.2 and 4.4: SD ancestor, we will ignore it
//
//
//
// notes from BCM2835 ARM Peripherals documentation. p 65
//


// - Command execution is commenced by writing the command plus the appropriate flags to the
//   CMDTM register after loading any required argument into the ARG1 register. The EMMC
//   module calculates the CRC checksum, transfers the command to the card, receives the
//   response and checks its CRC. Once the command has executed or timed-out bit 0 of register
//   INTERRUPT will be set. Please note that the INTERRUPT register is not self clearing, so the
//   software has first to reset it by writing 1 before using it to detect if a command has finished.

static void __dump_sdhost_infos(void)
{
    const uint32_t status = mmio_read(REG__SDCARD_STATUS);
    mini_uart_kernel_log("sdcard.status.dat_active: %u", !!(status & SDCARD_STATUS_DAT_ACTIVE));
}

static void __dump_sdhost_registers(const char *description)
{
    mini_uart_kernel_log("register state: %s:", description);
    mini_uart_kernel_log("REG__SDCARD_ARG2           : 0x%x // ACMD23 Argument", mmio_read(0x20300000u));  // ACMD23 Argument
    mini_uart_kernel_log("REG__SDCARD_BLKSIZECNT     : 0x%x // Block size and count", mmio_read(0x20300004u));  // Block size and count
    mini_uart_kernel_log("REG__SDCARD_ARG1           : 0x%x // Argument", mmio_read(0x20300008u));  // Argument
    mini_uart_kernel_log("REG__SDCARD_CMDTM          : 0x%x // Command and Transfer Mode", mmio_read(0x2030000Cu));  // Command and Transfer Mode
    mini_uart_kernel_log("REG__SDCARD_RESP0          : 0x%x // Response buts 031:000", mmio_read(0x20300010u));  // Response buts 031:000
    mini_uart_kernel_log("REG__SDCARD_RESP1          : 0x%x // Response buts 063:032", mmio_read(0x20300014u));  // Response buts 063:032
    mini_uart_kernel_log("REG__SDCARD_RESP2          : 0x%x // Response buts 095:064", mmio_read(0x20300018u));  // Response buts 095:064
    mini_uart_kernel_log("REG__SDCARD_RESP3          : 0x%x // Response buts 127:096", mmio_read(0x2030001Cu));  // Response buts 127:096
    mini_uart_kernel_log("REG__SDCARD_DATA           : 0x%x // Data", mmio_read(0x20300020u));  // Data
    mini_uart_kernel_log("REG__SDCARD_STATUS         : 0x%x // Status", mmio_read(0x20300024u));  // Status
    mini_uart_kernel_log("REG__SDCARD_CONTROL0       : 0x%x // Host Configuration bits", mmio_read(0x20300028u));  // Host Configuration bits
    mini_uart_kernel_log("REG__SDCARD_CONTROL1       : 0x%x // Host Configuration bits", mmio_read(0x2030002Cu));  // Host Configuration bits
    mini_uart_kernel_log("REG__SDCARD_INTERRUPT      : 0x%x // Interrupt Flags", mmio_read(0x20300030u));  // Interrupt Flags
    mini_uart_kernel_log("REG__SDCARD_IRPT_MASK      : 0x%x // Interrupt Flag Enable", mmio_read(0x20300034u));  // Interrupt Flag Enable
    mini_uart_kernel_log("REG__SDCARD_IRPT_EN        : 0x%x // Interrupt Generation Enable", mmio_read(0x20300038u));  // Interrupt Generation Enable
    mini_uart_kernel_log("REG__SDCARD_CONTROL2       : 0x%x // Host Configuration bits", mmio_read(0x2030003Cu));  // Host Configuration bits
    mini_uart_kernel_log("REG__SDCARD_FORCE_IRPT     : 0x%x // Force Interrupt Event", mmio_read(0x20300050u));  // Force Interrupt Event
    mini_uart_kernel_log("REG__SDCARD_BOOT_TIMEOUT   : 0x%x // Timeout in boot mode", mmio_read(0x20300070u));  // Timeout in boot mode
    mini_uart_kernel_log("REG__SDCARD_DBG_SEL        : 0x%x // Debug Bus Configuration", mmio_read(0x20300074u));  // Debug Bus Configuration
    mini_uart_kernel_log("REG__SDCARD_EXRDFIFO_EN    : 0x%x // Extension FIFO Enable", mmio_read(0x20300084u));  // Extension FIFO Enable
    mini_uart_kernel_log("REG__SDCARD_TUNE_STEP      : 0x%x // Delay per card clock tuning step", mmio_read(0x20300088u));  // Delay per card clock tuning step
    mini_uart_kernel_log("REG__SDCARD_TUNE_STEP_STD  : 0x%x // Card clock tuning steps for SDR", mmio_read(0x2030008Cu));  // Card clock tuning steps for SDR
    mini_uart_kernel_log("REG__SDCARD_TUNE_STEPS_DDR : 0x%x // Card clock tuning steps for DDR", mmio_read(0x20300090u));  // Card clock tuning steps for DDR
    mini_uart_kernel_log("REG__SDCARD_SPI_INT_SPT    : 0x%x // SPI Interrupt Support", mmio_read(0x203000F0u));  // SPI Interrupt Support
    mini_uart_kernel_log("REG__SDCARD_SLOT_ISR_VER   : 0x%x // Slot Interrupt Status and Version", mmio_read(0x203000FCu));  // Slot Interrupt Status and Version
}

static void _sdhost_wait_for_cmd_done(void)
{
    int counter = 0;
    for (;;) {
        const uint32_t interupt = mmio_read(REG__SDCARD_INTERRUPT);
        if (SDCARD_INTERUPT_CMD_DONE & interupt) {
            mini_uart_kernel_log("sdcard: CMD was done");
            mmio_write(REG__SDCARD_INTERRUPT, 0x1); // reset interupt register
            break;
        }
        if (interupt & SDCARD_INTERUPT_CTO_ERR) {
            mini_uart_kernel_log("sdcard: timeout on commmand line");
        }
        if (interupt & SDCARD_INTERUPT_CTO_ERR) {
            mini_uart_kernel_log("sdcard: command crc error");
        }
        counter++;
        cpu_delay(150);
        if (!(counter & 0xFF)) {
           __dump_sdhost_registers("during polling");
           if (counter > 2048)
               kernel_fatal_error("failed to wait for a commmand");
       }
    }
}

// type of expected response from card
#define SDCARD_CMDTM_RESP_TYPE_NONE            0x0u //  no response
#define SDCARD_CMDTM_RESP_TYPE_136_BITS        0x1u //  136 bits response
#define SDCARD_CMDTM_RESP_TYPE_48_BITS         0x2u //  48 bits response
#define SDCARD_CMDTM_RESP_TYPE_48_BITS_BUSY    0x3u //  48 bits response using busy

// 
#define SDCARD_CMDTM_DIR_HOST_TO_CARD 0x0u
#define SDCARD_CMDTM_DIR_CARD_TO_HOST 0x1u

static void _sdhost_send_command(
    uint8_t cmd_index,
    uint8_t is_data,
    uint8_t resp_type,
    uint8_t direction)
{
    mini_uart_kernel_log("sdcard: send CMD%u to sdcard", cmd_index);
    uint32_t cmdtm = 0u;

    cmdtm |= (cmd_index & 0x3Fu) << 24;  // CMD_INDEX:       29:24
    cmdtm |= ((!!is_data) << 21);        // CMD_ISDATA:      21
    cmdtm |= (0x3u & resp_type) << 16;   // CMD_RSPNS_TYPE:  17:16
    cmdtm |= ((!!direction) << 4);       // TM_DAT_DIR:      4

    // if a response is expected, check CRC and cmd index
    if (resp_type != SDCARD_CMDTM_RESP_TYPE_NONE)
    {
        cmdtm |= (1u << 19); // CMD_CRCCHK_EN:  19, check the response CRC
        cmdtm |= (1u << 20); // CMD_IXCHK_EN: 20, check that the response has same index as command
    }

    mmio_write(REG__SDCARD_CMDTM, cmdtm);
}

//  Public API
//

void sdhost_init(void)
{
    mini_uart_kernel_log("sdcar: initialize controller");
    __dump_sdhost_registers("at init");

    // RESET
    const uint32_t mask = (1u << 24);
    mmio_write(REG__SDCARD_CONTROL1, mask);
    while ((mmio_read(REG__SDCARD_CONTROL1) & mask)) {
        mini_uart_kernel_log("sdcard: wait for device (RESET)");
        asm("nop");
    }
    mini_uart_kernel_log("sdcard: RESET DONE:");
    __dump_sdhost_registers("after reset");

    // disable all SD interupts
    mmio_write(REG__SDCARD_IRPT_EN, 0x0u);
    __dump_sdhost_registers("after interupt disabled");

    // enabled irpt mask bit so interupt register is updated
    mmio_write(REG__SDCARD_IRPT_MASK, 0xFFFFFFFFu);

    // GPIO config

    // Select appropriate alt functions for pins 47:53
    // GPIO 47 : SD_CARD_DET (Detection) -> Alt 3
    // GPIO 48 : SD_CLK (Clock) -> Alt 3
    // GPIO 49 : SD_CMD (Command) -> Alt 3
    // GPIO 50 à 53 : SD_DAT0 à SD_DAT3 (Data) -> Alt 3
    for (int i = 47; i <= 53; i++)
    {
        gpio_select_function(i, GPIO_F_ALT_3);
    }

    // Select Mode
    gpio_set_pin_mode(47, GPIO_MODE_PULL_UP);
    gpio_set_pin_mode(48, GPIO_MODE_PULL_DOWN);
    gpio_set_pin_mode(49, GPIO_MODE_PULL_UP);
    gpio_set_pin_mode(50, GPIO_MODE_PULL_UP);
    gpio_set_pin_mode(51, GPIO_MODE_PULL_UP);
    gpio_set_pin_mode(52, GPIO_MODE_PULL_UP);
    gpio_set_pin_mode(53, GPIO_MODE_PULL_UP);
    cpu_delay(2048);

    // Congigure clock to 400khz
    //
    // base clock = 100Mhz
    // divider = base / target
    // thus: divider = 250 = 0xFA
    // also, set timeout = max_timeout = 0xE
    // TODO: document INTLEN
    const uint32_t clk_ctl1 = (0xFA << 8) | (0xE << 16) | SDCARD_CONTROL1_INTLEN;
    mmio_write(REG__SDCARD_CONTROL1, clk_ctl1);
    __dump_sdhost_registers("after clock configured");

    cpu_delay(8048);

    mmio_write(REG__SDCARD_CONTROL1, clk_ctl1 | SDCARD_CONTROL1_CLK_EN);

    cpu_delay(248);

    // send CMD0: GOTO IDLE state
    mmio_write(REG__SDCARD_ARG1, 0x0u);
    _sdhost_send_command(0u, 0u, SDCARD_CMDTM_RESP_TYPE_NONE, SDCARD_CMDTM_DIR_HOST_TO_CARD);

    __dump_sdhost_registers("after CMD0 was sent");
    _sdhost_wait_for_cmd_done();

    // send CMD8: check if high capacity memory
    //
    // check voltage, send an arbitrary pattern
    mmio_write(REG__SDCARD_ARG1, 0x000001AAu);  // 0x1: 2.7-3.6V / 0xAA: the pattern
    _sdhost_send_command(0x8u, 0, SDCARD_CMDTM_RESP_TYPE_48_BITS, SDCARD_CMDTM_DIR_HOST_TO_CARD);
    _sdhost_wait_for_cmd_done();

    // read back pattern: check correct echo. CARD agree on voltage and return the pattern.
    // So it is a SDHC/SDXC card. An old SDCARD 1.0 would have timedout
    // TODO: check or handle 1.0 card (if not too complicated)
    const uint32_t response = mmio_read(REG__SDCARD_RESP0);
    KERNEL_ASSERT(response == 0x1AAu);

}
