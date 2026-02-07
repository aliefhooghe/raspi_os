#ifndef SATAN_HARDWARE_SD_HOST_INTERNALS_H_
#define SATAN_HARDWARE_SD_HOST_INTERNALS_H_

////////////
//
// SD HOST registers layout
//
// 

// Status register: REG__SDCARD_HOST_STATUS
//
#define SDCARD_HOST_STATUS_DAT_LEVEL1_MASK          0x1E000000u  // value of data lines DAT7 to DAT4
#define SDCARD_HOST_STATUS_CMD_LEVEL_MASK           0x01000000u  // value of command line CMD
#define SDCARD_HOST_STATUS_DAT_LEVEL0_MASK          0x00F00000u  // value of data lines DAT3 to DAT0
#define SDCARD_HOST_STATUS_READ_TRANSFER_MASK       0x00000200u  // new data can be read from EMMC
#define SDCARD_HOST_STATUS_WRITE_TRANSFER_MASK      0x00000100u  // new data can be written to EMMC
#define SDCARD_HOST_STATUS_DAT_ACTIVE               0x00000004u  // at least one data line is active
#define SDCARD_HOST_STATUS_DAT_INHIBIT              0x00000002u  // data lines still used by previous data transfer
#define SDCARD_HOST_STATUS_CMD_INHIBIT              0x00000001u  // command line still used by previous command

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

// Block size and count
#define SDCARD_BLKSIZECNT_BLKSIZE              0x00000FFFu  // Block size
#define SDCARD_BLKSIZECNT_CNT_LSB              16

////////////
//
// SDHOST commands (CMD), applications commands (ACMD), and send parameters:
//
//

// sd command indexes
typedef enum {
    SDCARD_CMD_GO_IDLE_STATE                = 0,    // CMD0   : Reset card to idle state
    SDCARD_CMD_ALL_SEND_CID                 = 2,    // CMD2   : Ask all cards to send CID
    SDCARD_CMD_SEND_RELATIVE_ADDR           = 3,    // CMD3   : Ask card to publish RCA
    SDCARD_CMD_SELECT_DESELECT              = 7,    // CMD7   : Select or deselect a card
    SDCARD_CMD_SEND_IF_COND                 = 8,    // CMD8   : Send interface condition
    SDCARD_CMD_SEND_CSD                     = 9,    // CMD9   : Read Card-Specific Data
    SDCARD_CMD_SEND_STATUS                  = 13,   // CMD13  : Read card status register
    SDCARD_CMD_SET_BLOCKLEN                 = 16,   // CMD16  : Set block length (SDSC only)
    SDCARD_CMD_READ_SINGLE_BLOCK            = 17,   // CMD17  : Read a single data block
    SDCARD_CMD_WRITE_SINGLE_BLOCK           = 24,   // CMD24  : Write a single data block
    SDCARD_CMD_APP                          = 55,   // CMD55  : Prefix for application commands
} sdcard_cmd_t;

// sdcard application commands
typedef enum {
    SDCARD_ACMD_SET_BUS_WIDTH               = 6,    // ACMD6  : Set data bus width
    SDCARD_ACMD_SD_SEND_OP_COND             = 41,   // ACMD41 : Send operating condition (init)
    SDCARD_ACMD_SEND_SCR                    = 51,   // ACMD51 : Read SD Configuration Register
} sdcard_acmd_t;

// type of expected response from card
typedef enum {
    SDCARD_CMDTM_RESP_TYPE_NONE             = 0x0u, //  no response
    SDCARD_CMDTM_RESP_TYPE_136_BITS         = 0x1u, //  136 bits response
    SDCARD_CMDTM_RESP_TYPE_48_BITS          = 0x2u, //  48 bits response
    SDCARD_CMDTM_RESP_TYPE_48_BITS_BUSY     = 0x3u, //  48 bits response using busy (wait DAT0 goto 0 before DONE)
} sdcard_cmd_resp_type_t;

// command direction
typedef enum {
    SDCARD_CMDTM_DIR_HOST_TO_CARD           = 0x0u,
    SDCARD_CMDTM_DIR_CARD_TO_HOST           = 0x1u,
} sdcard_cmd_dir_t;

////////////
//
// SDCARD command responses layout
//
//

// ACMD41 response bits
// - high capacity card detection
// - confirm card is powered up
// - confirm voltage negotiation
#define SDCARD_ACMD41_RESP_HIGH_CAPACITY       0x40000000u  // card is SDHC/SDXC
#define SDCARD_ACMD41_RESP_POWER_UP            0x80000000u  // card is powered up
// also bit 23:15  -> voltage bits

// SEND RCA response format (CMD3): RCA at [31:16]
#define SDCARD_RCA_LSB   16u
#define SDCARD_RCA_MASK  0xFFFF0000u

//
// sdcard internal status,  CMD13, CMD7, etc. responses
#define SDCARD_STATUS_ERROR_FLAGS              0xFF000000u
#define SDCARD_STATUS_CURRENT_STATE_LSB        9u
#define SDCARD_STATUS_CURRENT_STATE_MASK       0x00001E00u
typedef enum {
    SDCARD_STATUS_CURRENT_STATE_IDLE       = 0x0u,
    SDCARD_STATUS_CURRENT_STATE_READY      = 0x1u,
    SDCARD_STATUS_CURRENT_STATE_IDENT      = 0x2u,
    SDCARD_STATUS_CURRENT_STATE_STDBY      = 0x3u,
    SDCARD_STATUS_CURRENT_STATE_TRANSFERT  = 0x4u,
} sdcard_status_current_state_t;

#define SDCARD_STATUS_READY_FOR_DATA           0x00000100u  // card is ready for data transfert
#define SDCARD_STATUS_APP_CMD                  0x00000020u  // card is awaiting an application command (ACMD)

#endif
