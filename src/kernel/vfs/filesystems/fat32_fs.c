
#include <stdint.h>
#include "fat32_fs.h"
#include "hardware/mini_uart.h"
#include "lib/str.h"
#include "vfs/inode.h"


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
    uint32_t table_size_32;          // size in sector
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

typedef struct __attribute__((packed))
{
    char filename[8];                // filename padded with space
    char file_extension[3];          // file extension
    uint8_t attributes;              // attributes (read only, hidden, ..)
    uint8_t reserved;                // reserved for windows NT
    uint8_t creation_time_ms;        // creation date (1/10 of seconds)
    uint16_t creation_time;          // creation hour
    uint16_t creation_date;          // creation date
    uint16_t last_access_date;       //
    uint16_t starting_cluster_high;  //
    uint16_t last_write_time;        // last modification hour
    uint16_t last_write_date;        // last modification date
    uint16_t starting_cluster_low;   //
    uint32_t file_size;              // file size in bytes
} fat_directory_entry_t;

_Static_assert(
    sizeof(fat_directory_entry_t) == 32,
    "Fat32 directory entrry is required to be a 32 byte struct"
);

typedef struct __attribute__((packed))
{
    uint8_t sequence_number;          // Sequence number and "Last-Logical-Entry" flag
    uint16_t name_part1[5];           // Chars 1-5 (UTF-16)
    uint8_t attributes;               // Always 0x0F
    uint8_t type;                     // Always 0x00 for LFN
    uint8_t checksum;                 // Checksum of the SFN 8.3 name
    uint16_t name_part2[6];           // Chars 6-11 (UTF-16)
    uint16_t first_cluster;           // Always 0x0000
    uint16_t name_part3[2];           // Chars 12-13 (UTF-16)
} fat_lfn_entry_t;

//
// Fat Directory Entry attributes
// struct field fat_directory_entry_t->attributes
// 
#define FAT_DIR_ENTRY_ATTR_READ_ONLY 0x01
#define FAT_DIR_ENTRY_ATTR_HIDDEN    0x02
#define FAT_DIR_ENTRY_ATTR_SYSTEM    0x04
#define FAT_DIR_ENTRY_ATTR_VOLUME_ID 0x08
#define FAT_DIR_ENTRY_ATTR_DIRECTORY 0x10
#define FAT_DIR_ENTRY_ATTR_ARCHIVE   0x20
#define FAT_DIR_ENTRY_ATTR_LFN       0x0F   // Long File Name entry. To be checked first (contains other attributes)

//
// Helpers
//
static int _is_fat32_boot_sector(fat_boot_sector_t *bs)
{
    return (bs->table_size_16 == 0u);
}

// compute the index of the first data sector
static uint32_t _fat32_data_area_sector_index(
    fat_boot_sector_t *bs,
    fat_extended_boot_sector32_t *ebs)
{
    return bs->reserved_sector_count + (bs->fat_count * ebs->table_size_32);
}

// convert a logical cluster number into into a physical sector address
static uint32_t _fat32_cluster_sector_index(
    fat_boot_sector_t *bs,
    fat_extended_boot_sector32_t *ebs,
    uint32_t cluster_num)
{
    // cluster are indexed from 2
    return _fat32_data_area_sector_index(bs, ebs) +
           (cluster_num - 2u) * bs->sectors_per_cluster;
}

//
// Public FAT32 filesystem API
// 
super_block_t *fat32_create_filesystem(block_device_t *blk_dev)
{
    (void)blk_dev;
    return NULL;
}


void test_fat32(const uint8_t *fat32_base, size_t data_size)
{
    mini_uart_kernel_log("[FAT32] start Fat32 test procedure!");

    //
    // read boot sector
    // fat_boot_sector_t *fat32_base = (fat_boot_sector_t*)data;
    fat_boot_sector_t *boot_sector = (fat_boot_sector_t*)fat32_base;

    char oem_name[9];
    _memcpy(oem_name, boot_sector->oem_name, 8);
    oem_name[8] = '\0';

    mini_uart_kernel_log("[FAT32] boot_sector.boot_jmp              = [%x, %x, %x]", boot_sector->boot_jmp[0], boot_sector->boot_jmp[1], boot_sector->boot_jmp[2]);
    mini_uart_kernel_log("[FAT32] boot_sector.oem_name              = '%s'", oem_name);
    mini_uart_kernel_log("[FAT32] boot_sector.bytes_per_sector      = 0x%x", boot_sector->bytes_per_sector);
    mini_uart_kernel_log("[FAT32] boot_sector.sectors_per_cluster   = 0x%x", boot_sector->sectors_per_cluster);
    mini_uart_kernel_log("[FAT32] boot_sector.reserved_sector_count = 0x%x", boot_sector->reserved_sector_count);
    mini_uart_kernel_log("[FAT32] boot_sector.table_count           = 0x%x", boot_sector->fat_count);
    mini_uart_kernel_log("[FAT32] boot_sector.root_entry_count      = 0x%x", boot_sector->root_entry_count);
    mini_uart_kernel_log("[FAT32] boot_sector.total_sectors_16      = 0x%x", boot_sector->total_sectors_16);
    mini_uart_kernel_log("[FAT32] boot_sector.media_type            = 0x%x", boot_sector->media_descriptor_type);
    mini_uart_kernel_log("[FAT32] boot_sector.table_size_16         = 0x%x", boot_sector->table_size_16);
    mini_uart_kernel_log("[FAT32] boot_sector.sectors_per_track     = 0x%x", boot_sector->sectors_per_track);
    mini_uart_kernel_log("[FAT32] boot_sector.head_side_count       = 0x%x", boot_sector->head_side_count);
    mini_uart_kernel_log("[FAT32] boot_sector.hidden_sector_count   = 0x%x", boot_sector->hidden_sector_count);
    mini_uart_kernel_log("[FAT32] boot_sector.total_sectors_32      = 0x%x", boot_sector->total_sectors_32);

    mini_uart_kernel_log("[FAT32] is fat32: %u", _is_fat32_boot_sector(boot_sector));

    const size_t sector_count = (
        boot_sector->total_sectors_32 == 0 ?
            boot_sector->total_sectors_16 :
            boot_sector->total_sectors_32);

    mini_uart_kernel_log("[FAT32] sector count = %u", sector_count);

    //
    // read fat32 extended boot sector
    fat_extended_boot_sector32_t *ebs = (fat_extended_boot_sector32_t*)(&boot_sector[1]);

    mini_uart_kernel_log("[FAT32] ext bs.table_size_32    = %x", ebs->table_size_32);
    mini_uart_kernel_log("[FAT32] ext bs.extended_flags   = %x", ebs->extended_flags);
    mini_uart_kernel_log("[FAT32] ext bs.fat_version      = %x", ebs->fat_version);
    mini_uart_kernel_log("[FAT32] ext bs.root_cluster     = %x", ebs->root_cluster);
    mini_uart_kernel_log("[FAT32] ext bs.fat_info         = %x", ebs->fat_info);
    mini_uart_kernel_log("[FAT32] ext bs.backup_BS_sector = %x", ebs->backup_BS_sector);
    mini_uart_kernel_log("[FAT32] ext bs.drive_number     = %x", ebs->drive_number);
    mini_uart_kernel_log("[FAT32] ext bs.reserved_1       = %x", ebs->reserved_1);
    mini_uart_kernel_log("[FAT32] ext bs.boot_signature   = %x", ebs->boot_signature);
    mini_uart_kernel_log("[FAT32] ext bs.volume_id        = %x", ebs->volume_id);

    //
    // Locate the first sector of the root dictory cluster
    // 
    const uint32_t root_cluster_sector =
        _fat32_cluster_sector_index(boot_sector, ebs, ebs->root_cluster);

    // warning: the root cluster may be fragmented
    mini_uart_kernel_log("[FAT32] ## read fs root @ sector %u ##", root_cluster_sector);

    fat_directory_entry_t *entry = (fat_directory_entry_t*)(
        fat32_base + root_cluster_sector * boot_sector->bytes_per_sector);

    for (; entry->filename[0] != 0x0u; entry++) {
        if (entry->filename[0] == 0xE5u) {
            // skip deleted file
            continue;
        }

        // retrieve entry name as a C string
        char entry_name[9];
        char extension[4];
        
        _memcpy(entry_name, entry->filename, 8);
        _memcpy(extension, entry->file_extension, 3);

        entry_name[8] = '\0';
        extension[3] = '\0';

        // check for Long File Name entry first
        if (entry->attributes & FAT_DIR_ENTRY_ATTR_LFN) {
            mini_uart_kernel_log("[FAT32] skip LFN entry: %s.%s", entry_name, extension);
            continue;
        }

        // check if the entry is a volume ID
        if (entry->attributes & FAT_DIR_ENTRY_ATTR_VOLUME_ID) {
            mini_uart_kernel_log("[FAT32] volume id: %s", entry_name);
            continue;
        }

        if (entry->attributes & FAT_DIR_ENTRY_ATTR_DIRECTORY) {
            mini_uart_kernel_log("[FAT32] short name directory: %s.%s", entry_name, extension);
        }
        else {
            mini_uart_kernel_log("[FAT32] short name file: %s.%s size=%u", entry_name, extension, entry->file_size);

            const uint32_t first_file_cluster = (
                (uint32_t)entry->starting_cluster_high << 16) | (entry->starting_cluster_low);
            const uint32_t first_file_sector = _fat32_cluster_sector_index(boot_sector, ebs, first_file_cluster);
            const uint8_t *file_data = fat32_base + boot_sector->bytes_per_sector * first_file_sector;
            char buffer[64];

            _memcpy(buffer, file_data, entry->file_size);
            buffer[entry->file_size] = 0;

            mini_uart_kernel_log("[FAT32] content: %s\n", buffer);
        }
    }

    mini_uart_kernel_log("[FAT32] test procedure: DONE.");
}
