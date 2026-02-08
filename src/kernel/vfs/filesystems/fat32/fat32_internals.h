#ifndef SATAN_VFS_FAT32_INTERNALS_H_
#define SATAN_VFS_FAT32_INTERNALS_H_

#include <stdint.h>

//
// FAT32 internal data structures
//

typedef struct __attribute__((packed))
{
    uint8_t boot_jmp[3];             //
    char oem_name[8];                // implementation name
    uint16_t bytes_per_sector;       //
    uint8_t sectors_per_cluster;     //
    uint16_t reserved_sector_count;  //
    uint8_t fat_count;               // number of fat table replica (often 2)
    uint16_t root_entry_count;       // unused for fat32
    uint16_t total_sectors_16;       //
    uint8_t media_descriptor_type;   // 
    uint16_t table_size_16;          // nb of sector per FAT. FAT12/16 only
    uint16_t sectors_per_track;      //
    uint16_t head_side_count;        //
    uint32_t hidden_sector_count;    //
    uint32_t total_sectors_32;       // set if sector count is > 65535
} fat_boot_sector_t;

typedef struct __attribute__((packed))
{
    uint32_t table_size_32;          // FAT table size in sector
    uint16_t extended_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fat_info;
    uint16_t backup_BS_sector;
    uint8_t reserved_0[12];
    uint8_t drive_number;
    uint8_t reserved_1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];       // ="FAT32"
} fat_extended_boot_sector32_t;

// Fat Table entries layout
#define FAT_ENTRY_MASK                 0x0FFFFFFFu    // Fat32 table entries use the 28 LSbs
#define FAT_ENTRY_FREE_CLUSTER         0x00000000u    // Cluster is free
#define FAT_ENTRY_BEGIN                0x00000002u    // fat entries in this range are valid cluter numbers
#define FAT_ENTRY_END                  0x00FFFFF0u    //  .. from F0 to F6: reserved
#define FAT_ENTRY_BAD_CLUSTER          0x00FFFFF7u    // Cluster is bad
#define FAT_ENTRY_EOF_BEGIN            0x0FFFFFF8u    // from this value: end of cluster

typedef struct __attribute__((packed))
{
    char filename[8];                // filename padded with space
    char file_extension[3];          // file extension
    uint8_t attributes;              // attributes (read only, hidden, ..)
    uint8_t nt_reserved;             // reserved for windows NT (hidden bit: lowercase file)
    uint8_t creation_time_ms;        // creation date (1/10 of seconds)
    uint16_t creation_time;          // creation hour
    uint16_t creation_date;          // creation date
    uint16_t last_access_date;       //
    uint16_t starting_cluster_high;  //
    uint16_t last_write_time;        // last modification hour
    uint16_t last_write_date;        // last modification date
    uint16_t starting_cluster_low;   //
    uint32_t file_size;              // file size in bytes
} fat_sfn_directory_entry_t;

#define FAT_SFN_ENTRY_NTRES_NAME_LOWER  0x08u  // set if SFN name should be in lowercase
#define FAT_SFN_ENTRY_NTRES_EXT_LOWER   0x10u  // set if SFN extension should be in lowercase

typedef struct __attribute__((packed))
{
    uint8_t sequence;                // sequence number | LastLogicalEntry flag
    uint16_t name1[5];               // name part1: 5 utf16 chars
    uint8_t attributes;              // attributes: Always 0xF for LFN
    uint8_t type;                    // always zero
    uint8_t checksum;                // Short File Name checksum
    uint16_t name2[6];               // name part2: 6 utf16 chars
    uint16_t cluster;                // always zero
    uint16_t name3[2];               // name part3: 2 utf16 chars
} fat_lfn_directory_entry_t;

//
// LFN entryies sequence layout
#define FAT_LFN_CHAR_COUNT              13u    // 2 + 6 + 5 = number of char per LFN
#define FAT_LFN_SEQ_LAST_LONG_ENTRY     0x40u  // if set: this is the last lfn entry (the first from offset pov)
#define FAT_LFN_SEQ_NUM_MASK            0x1F   // sequence num mask. Go to

// ensure that we actually match the expected size
_Static_assert(
    sizeof(fat_sfn_directory_entry_t) == 32,
    "Fat32 Short File Name directory entry is required to be a 32 byte struct"
);
_Static_assert(
    sizeof(fat_lfn_directory_entry_t) == 32,
    "Fat32 Long File Name directory entry is required to be a 32 byte struct"
);


//
// Fat Directory Entry attributes
// struct field fat_directory_entry_t->attributes
#define FAT_DIR_ENTRY_ATTR_READ_ONLY 0x01
#define FAT_DIR_ENTRY_ATTR_HIDDEN    0x02
#define FAT_DIR_ENTRY_ATTR_SYSTEM    0x04
#define FAT_DIR_ENTRY_ATTR_VOLUME_ID 0x08
#define FAT_DIR_ENTRY_ATTR_DIRECTORY 0x10
#define FAT_DIR_ENTRY_ATTR_ARCHIVE   0x20
#define FAT_DIR_ENTRY_ATTR_LFN       0x0F   // Long File Name entry. To be checked first (contains other attributes)

#endif
