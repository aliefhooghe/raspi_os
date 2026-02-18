
#include <stdint.h>

#include "hardware/cpu.h"
#include "hardware/gpio.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmio.h"
#include "kernel.h"

#include "sd_host.h"
#include "sd_host_internals.h"

//
// notes from BCM2835 ARM Peripherals documentation. p 65
//
// - Command execution is commenced by writing the command plus the appropriate flags to the
//   CMDTM register after loading any required argument into the ARG1 register. The EMMC
//   module calculates the CRC checksum, transfers the command to the card, receives the
//   response and checks its CRC. Once the command has executed or timed-out bit 0 of register
//   INTERRUPT will be set. Please note that the INTERRUPT register is not self clearing, so the
//   software has first to reset it by writing 1 before using it to detect if a command has finished.


static void _sdhost_wait_for_cmd_done(void)
{
    int counter = 0;
    for (;;) {
        const uint32_t interupt = mmio_read(REG__SDCARD_INTERRUPT);
        if (SDCARD_INTERUPT_CMD_DONE & interupt) {
            mini_uart_kernel_log("sdcard: CMD was done");

            // we should read response now in case we are interupted
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
           if (counter > 2048)
               kernel_fatal_error("failed to wait for a commmand");
       }
    }
}

static void _sdhost_enable_clock(uint16_t freq_khz)
{
    // Configure clock:
    // base clock = 100Mhz = 100000Khz
    // divider = base / target (10 bits)

    const uint16_t divider = 100000u / freq_khz;
    const uint8_t timeout = 0xEu; // max_timeout

    const uint32_t clk_ctl1 = (
        (divider & 0xFF) << 8) |
        ((divider & 0x300) >> 2) |
        (timeout << 16) |
        SDCARD_CONTROL1_INTLEN;

    mmio_write(REG__SDCARD_CONTROL1, clk_ctl1);
    cpu_delay(8048);
    mmio_write(REG__SDCARD_CONTROL1, clk_ctl1 | SDCARD_CONTROL1_CLK_EN);
}

static void _sdhost_configure_blk_size_count(uint32_t block_size, uint32_t block_count)
{
    mmio_write(
        REG__SDCARD_BLKSIZECNT,
        (SDCARD_BLKSIZECNT_BLKSIZE & block_size) |
        (block_count << SDCARD_BLKSIZECNT_CNT_LSB));
}

static uint32_t _sdhost_cmdtm_val(
    uint8_t cmd,
    sdcard_cmd_resp_type_t resp_type,
    sdcard_cmd_dir_t direction)
{
    uint32_t cmdtm = 0u;

    cmdtm |= (cmd & 0x3Fu) << 24;        // CMD_INDEX:       29:24
    cmdtm |= (0x3u & resp_type) << 16;   // CMD_RSPNS_TYPE:  17:16
    cmdtm |= ((!!direction) << 4);       // TM_DAT_DIR:      4

    // if a response is expected, check CRC and cmd index
    if (resp_type != SDCARD_CMDTM_RESP_TYPE_NONE)
    {
        // it *may* append that 48_BIT_BUSY resp have a bad CRC ??
        cmdtm |= (1u << 19); // CMD_CRCCHK_EN:  19, check the response CRC
        cmdtm |= (1u << 20); // CMD_IXCHK_EN: 20, check that the response has same index as command
    }

    return cmdtm;
}

static void _sdhost_send_data_command(
    uint8_t cmd,
    sdcard_cmd_resp_type_t resp_type,
    sdcard_cmd_dir_t direction)
{
    mini_uart_kernel_log("sdcard: send CMD%u to sdcard", cmd);
    uint32_t cmdtm = _sdhost_cmdtm_val(cmd, resp_type, direction);
    cmdtm |= (1 << 21);        // CMD_ISDATA:      21
    mmio_write(REG__SDCARD_CMDTM, cmdtm);
}

static void _sdhost_send_command(
    uint8_t cmd,
    sdcard_cmd_resp_type_t resp_type,
    sdcard_cmd_dir_t direction)
{
    mini_uart_kernel_log("sdcard: send CMD%u to sdcard", cmd);
    uint32_t cmdtm = _sdhost_cmdtm_val(cmd, resp_type, direction);
    mmio_write(REG__SDCARD_CMDTM, cmdtm);
}

//
//  Public API
//

void sdhost_init(void)
{
    mini_uart_kernel_log("sdcard: initialize controller");

    // RESET
    const uint32_t mask = (1u << 24);
    mmio_write(REG__SDCARD_CONTROL1, mask);
    while ((mmio_read(REG__SDCARD_CONTROL1) & mask)) {
        mini_uart_kernel_log("sdcard: wait for device (RESET)");
        asm("nop");
    }
    mini_uart_kernel_log("sdcard: RESET DONE:");

    // disable all SD interupts
    mmio_write(REG__SDCARD_IRPT_EN, 0x0u);

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
    _sdhost_enable_clock(400);
    cpu_delay(248);

    // send CMD0: GOTO IDLE state
    mmio_write(REG__SDCARD_ARG1, 0x0u);
    _sdhost_send_command(
        SDCARD_CMD_GO_IDLE_STATE,
        SDCARD_CMDTM_RESP_TYPE_NONE, SDCARD_CMDTM_DIR_HOST_TO_CARD);
    _sdhost_wait_for_cmd_done();

    // send CMD8: check if high capacity memory
    //
    // check voltage, send an arbitrary pattern
    // TODO: clarify the mapping
    mmio_write(REG__SDCARD_ARG1, 0x000001AAu);  // 0x1: 2.7-3.6V / 0xAA: the pattern
    _sdhost_send_command(
        SDCARD_CMD_SEND_IF_COND,
        SDCARD_CMDTM_RESP_TYPE_48_BITS, SDCARD_CMDTM_DIR_HOST_TO_CARD);
    _sdhost_wait_for_cmd_done();

    // read back pattern: check correct echo. CARD agree on voltage and return the pattern.
    // So it is a SDHC/SDXC card. An old SDCARD 1.0 would have timedout
    // TODO: check or handle 1.0 card (if not too complicated)
    const uint32_t negotiation_resp = mmio_read(REG__SDCARD_RESP0);
    KERNEL_ASSERT(negotiation_resp == 0x1AAu);
    mini_uart_kernel_log("sdcard: negotiation is done.");

    // power up the card
    // Send application specific command 41
    // send SDCARD_C

    // send CMD_APP
    for (;;) {
        mmio_write(REG__SDCARD_ARG1, 0x0u);
        _sdhost_send_command(
            SDCARD_CMD_APP,
            SDCARD_CMDTM_RESP_TYPE_48_BITS, SDCARD_CMDTM_DIR_HOST_TO_CARD);
        _sdhost_wait_for_cmd_done();

        // send ACMD41
        mmio_write(REG__SDCARD_ARG1, 0x40FF8000); // TODO: document this value
        _sdhost_send_command(
            SDCARD_ACMD_SD_SEND_OP_COND,
            SDCARD_CMDTM_RESP_TYPE_48_BITS_BUSY, SDCARD_CMDTM_DIR_HOST_TO_CARD);
        _sdhost_wait_for_cmd_done();

        const uint32_t operation_condition = mmio_read(REG__SDCARD_RESP0);
        // here: 
        mini_uart_kernel_log("sdcard: response for ACMD41 is 0x%x", operation_condition );

        if (operation_condition & SDCARD_ACMD41_RESP_POWER_UP)
        {
            mini_uart_kernel_log("sdcard: card is powered up.");

            if (operation_condition & SDCARD_ACMD41_RESP_HIGH_CAPACITY)
            {
                mini_uart_kernel_log("sdcard: card type: SDHC/SDXC");
            }
            else
            {
                kernel_fatal_error("sdcard: card type SDSC is NOT supported.");
            }
            break;
        }
        else
        {
            cpu_delay(512);
        }
    }

    // Read Card Identification CID
    mmio_write(REG__SDCARD_ARG1, 0x0u);
    _sdhost_send_command(
        SDCARD_CMD_ALL_SEND_CID,
        SDCARD_CMDTM_RESP_TYPE_136_BITS, SDCARD_CMDTM_DIR_HOST_TO_CARD);
    _sdhost_wait_for_cmd_done();

    // TODO: extract response.
    const uint32_t resps[5] = {
        mmio_read(REG__SDCARD_RESP0),
        mmio_read(REG__SDCARD_RESP1),
        mmio_read(REG__SDCARD_RESP2),
        mmio_read(REG__SDCARD_RESP3),
        0
    };

    // 2. Le piège des registres RESP sur BCM2835
    // Il y a une subtilité propre au contrôleur EMMC de la Raspberry Pi : pour les réponses longues (R2), les bits
    // sont décalés de 8 bits vers la gauche car le contrôleur "pousse" les données en omettant l'octet d'en-tête de la
    // spécification SD.
    // C'est pour cela que votre "QEMU" (qui devrait être sur 4 octets alignés) peut paraître un peu étrange si
    // vous essayez de mapper la structure exacte de la spec SD directement sur les registres. Mais pour l'instant,
    // l'important est que vous recevez des données cohérentes.
    for (int i = 0; i < 4; i++) {
        mini_uart_kernel_log("sdcard: CID: RESP%d = 0x%x", i, resps[i]);
    }

    // Read RCA: Relative Card Address: CMD3
    mmio_write(REG__SDCARD_ARG1, 0x0u);
    _sdhost_send_command(
        SDCARD_CMD_SEND_RELATIVE_ADDR,
        SDCARD_CMDTM_RESP_TYPE_48_BITS, SDCARD_CMDTM_DIR_HOST_TO_CARD);
    _sdhost_wait_for_cmd_done();

    const uint32_t rca_resp = mmio_read(REG__SDCARD_RESP0);
    const uint32_t relative_card_address = (rca_resp & SDCARD_RCA_MASK) >> SDCARD_RCA_LSB;
    mini_uart_kernel_log("sdcard: Relative Card Address = 0x%x", relative_card_address);
    mini_uart_kernel_log("sdcard: card identification is done");

    // NOW: card is in STANDBY mode. We want to switch to TRANSFER MODE
    mmio_write(REG__SDCARD_ARG1, relative_card_address << SDCARD_RCA_LSB);
    _sdhost_send_command(
        SDCARD_CMD_SELECT_DESELECT,
        SDCARD_CMDTM_RESP_TYPE_48_BITS_BUSY, SDCARD_CMDTM_DIR_HOST_TO_CARD);
    _sdhost_wait_for_cmd_done();

    // TODO: resp0 is now the internal state of card WHEN it received the command. bit 12:9  (3 stby / 4 transfert)

    // Read card status: ensure it is in the correct state.
    mmio_write(REG__SDCARD_ARG1, relative_card_address << SDCARD_RCA_LSB);
    _sdhost_send_command(
        SDCARD_CMD_SEND_STATUS,
        SDCARD_CMDTM_RESP_TYPE_48_BITS, SDCARD_CMDTM_DIR_HOST_TO_CARD);
    _sdhost_wait_for_cmd_done();

    const uint32_t card_status = mmio_read(REG__SDCARD_RESP0);
    const sdcard_status_current_state_t state = (card_status & SDCARD_STATUS_CURRENT_STATE_MASK) >> SDCARD_STATUS_CURRENT_STATE_LSB;

    mini_uart_kernel_log("sdcard: sdcard status is %x", state);

    if (state == SDCARD_STATUS_CURRENT_STATE_TRANSFERT)
    {
        mini_uart_kernel_log("sdcard: card is ready for transfert");
    }

    // TODO: try to read the Master Boot Record
    // for now keep the bus clock to 400 Hz

    // configure block count and size: read blocks one by one to start
    // should be persistent until we change it (to perform multi block read)
    const uint32_t block_size = 512;
    const uint32_t block_count = 1u;
    _sdhost_configure_blk_size_count(block_size, block_count);
    mini_uart_kernel_log("sdcard: sd host driver is initialized");
}

int sdhost_read_block(uint32_t block_index, uint8_t block[512])
{
    mini_uart_kernel_log("sdcard: read block @ %u", block_index);

    // configure block index
    mmio_write(REG__SDCARD_ARG1, block_index);

    // send a read request
    _sdhost_send_data_command(
       SDCARD_CMD_READ_SINGLE_BLOCK, SDCARD_CMDTM_RESP_TYPE_48_BITS,
       SDCARD_CMDTM_DIR_CARD_TO_HOST);
    _sdhost_wait_for_cmd_done();

    // wait for data
    for (;;) {
        const int32_t interupt = mmio_read(REG__SDCARD_INTERRUPT);

        if (interupt & SDCARD_INTERUPT_READ_READY)
        {
            mini_uart_kernel_log("sdcard: data is ready");
            break;
        }
        else if (interupt & SDCARD_INTERUPT_DTO_ERR)
        {
            kernel_fatal_error("sdcard: read timeout");
        }
        else
        {
            cpu_delay(64);
        }
    }

    // read data 
    uint32_t *buffer = (uint32_t*)block;  // 128x4 = 512 bytes
    for (int i = 0; i < 128; i++) {
        buffer[i] = mmio_read(REG__SDCARD_DATA);
    }

    // reset interupt flags
    mmio_write(REG__SDCARD_INTERRUPT, SDCARD_INTERUPT_READ_READY);

    return 0;
}
