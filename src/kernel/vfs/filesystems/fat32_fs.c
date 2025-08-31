
#include <stdint.h>
#include "fat32_fs.h"


typedef struct __attribute__((packed))
{
    uint8_t boot_jmp[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t table_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t table_size_16;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t hidden_sector_count;
    uint32_t total_sectors_32;
} fat_boot_sector_t;

typedef struct __attribute__((packed))
{
    uint32_t table_size_32;
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
    uint8_t fat_type_label[8]; // ="FAT32"
} fat_extended_boot_sector32_t;

//
// Public FAT32 filesystem API
// 

super_block_t *fat32_create_filesystem(block_device_t *blk_dev)
{
    return NULL;
}
