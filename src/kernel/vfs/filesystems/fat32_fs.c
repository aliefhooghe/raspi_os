
#include <stddef.h>
#include <stdint.h>

#include "hardware/mini_uart.h"
#include "kernel.h"
#include "kernel_types.h"
#include "lib/str.h"
#include "memory/memory_allocator.h"
#include "vfs/device_ops.h"
#include "vfs/inode.h"
#include "vfs/super_block.h"  // IWYU pragma: keep

#include "fat32_fs.h"
#include "fat32_internals.h"

//
// Fat32 filesystem readonly implementation
//
// Note: fat32 does not have inode concept: we must build it on top of the underlying fat32 structures
// We are using the cluster offset to encode the inode.

#define FAT32_SECTOR_SIZE 512u       // we only handle 512 sectors

typedef struct {
    block_device_t *device;                // underlying block device

    uint32_t data_sector_index;            // data area sector index
    uint32_t sectors_per_cluster;          // nb of sector per cluster. Sector size is device->block_size
    uint32_t root_cluster_sector_index;    // sector index of root cluster
} fat32_sb_private_t;

//
// FAT32 Helpers
//
static int _is_fat32_boot_sector(const fat_boot_sector_t *bs)
{
    return (bs->table_size_16 == 0u);
}

// convert a logical cluster number into into a physical sector address
static uint32_t _fat32_cluster_sector_index(
    const fat32_sb_private_t *private,
    uint32_t cluster_num)
{
    return private ->data_sector_index +
        (cluster_num - 2u) * private->sectors_per_cluster;
}

static fat32_sb_private_t *_fat32_init_fs_private(block_device_t *device)
{
    uint8_t block[FAT32_SECTOR_SIZE];

    // only handle 512 bytes sectors
    KERNEL_ASSERT(device->block_size == FAT32_SECTOR_SIZE);

    // 
    // read the boot sector
    const int status = device->ops->read_block(device->private, 0u, block);
    KERNEL_ASSERT(status == 1u);

    const fat_boot_sector_t *boot_sector = (fat_boot_sector_t*)block;

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

    // TODO: handle error
    KERNEL_ASSERT(_is_fat32_boot_sector(boot_sector));

    const size_t sector_count = (
        boot_sector->total_sectors_32 == 0 ?
            boot_sector->total_sectors_16 :
            boot_sector->total_sectors_32);

    mini_uart_kernel_log("[FAT32] sector count = %u", sector_count);

    //
    // read fat32 extended boot sector
    fat_extended_boot_sector32_t *ebs = (fat_extended_boot_sector32_t*)(&boot_sector[1]);

    mini_uart_kernel_log("[FAT32] ext bs.table_size_32    = 0x%x", ebs->table_size_32);
    mini_uart_kernel_log("[FAT32] ext bs.extended_flags   = 0x%x", ebs->extended_flags);
    mini_uart_kernel_log("[FAT32] ext bs.fat_version      = 0x%x", ebs->fat_version);
    mini_uart_kernel_log("[FAT32] ext bs.root_cluster     = 0x%x", ebs->root_cluster);
    mini_uart_kernel_log("[FAT32] ext bs.fat_info         = 0x%x", ebs->fat_info);
    mini_uart_kernel_log("[FAT32] ext bs.backup_BS_sector = 0x%x", ebs->backup_BS_sector);
    mini_uart_kernel_log("[FAT32] ext bs.drive_number     = 0x%x", ebs->drive_number);
    mini_uart_kernel_log("[FAT32] ext bs.reserved_1       = 0x%x", ebs->reserved_1);
    mini_uart_kernel_log("[FAT32] ext bs.boot_signature   = 0x%x", ebs->boot_signature);
    mini_uart_kernel_log("[FAT32] ext bs.volume_id        = 0x%x", ebs->volume_id);

    // compute data area sector index (first data sector)
    const uint32_t data_aera_sector_index =
        boot_sector->reserved_sector_count +
        boot_sector->fat_count * ebs->table_size_32;

    // allocate fs private data
    fat32_sb_private_t* private = (fat32_sb_private_t*)memory_calloc(sizeof(fat32_sb_private_t));
    // TODO: more error handling
    KERNEL_ASSERT(private != NULL);

    private->device = device; // keep a reference to block device
    private->data_sector_index = data_aera_sector_index;
    private->sectors_per_cluster = boot_sector->sectors_per_cluster;

    // compute root cluster location.
    private->root_cluster_sector_index =
        _fat32_cluster_sector_index(private, ebs->root_cluster);

    return private;
}

//
// FAT32 VFS interface implementation
//

//
// FAT32 Regular file operations

// static ssize_t _fat32fs_reg_file_read(
//     file_t *file, void *data, size_t size)
// {
//     (void)file;
//     (void)data;
//     (void)size;
//     kernel_fatal_error(__FUNCTION__);
//     return -1;
// }

// static ssize_t _fat32fs_reg_file_write(
//     file_t *file, const void *data, size_t size)
// {
//     (void)file;
//     (void)data;
//     (void)size;
//     kernel_fatal_error(__FUNCTION__);
//     return -1;
// }

// static ssize_t _fat32fs_reg_file_seek(
//     file_t *file, int32_t offset, int32_t whence)
// {
//     (void)file;
//     (void)offset;
//     (void)whence;
//     kernel_fatal_error(__FUNCTION__);
//     return -1;
// }

// static const file_ops_t _fat32fs_reg_file_ops = {
//     .read = _fat32fs_reg_file_read,
//     .write = _fat32fs_reg_file_write,
//     .seek = _fat32fs_reg_file_seek,
//     .readdir = NULL
// };

//
// FAT32 Directory file operations

static int _fat32_fs_dir_readdir(
    file_t *file, struct dirent *entries, size_t count)
{
    mini_uart_kernel_log("fat32fs: dir_file_ops: readdir");
    inode_t *dir = file->inode;
    KERNEL_ASSERT(dir != NULL);

    super_block_t *sb = dir->super_block;
    fat32_sb_private_t *sb_private = (fat32_sb_private_t*)sb->private;
    block_device_t *device = sb_private->device;

    // read one sector from device
    const uint32_t dir_sector_index = dir->ino; // ino convention
    char sector[FAT32_SECTOR_SIZE];
    const int block_count = device->ops->read_block(device->private, dir_sector_index, sector);
    KERNEL_ASSERT(block_count == 1);

    // read dir
    size_t i = 0u;
    fat_directory_entry_t *entry = file->pos + (fat_directory_entry_t*)sector;

    for (;
        i < count && entry->filename[0] != 0x0u;
        entry++, file->pos++)
    {
        if (entry->filename[0] == 0xE5u) {
            // skip deleted file
            continue;
        }

        // retrieve entry name as a C string
        char entry_name[9];
        char extension[4];
        char filename[9 + 4 + 1];

        _memcpy(entry_name, entry->filename, 8);
        _memcpy(extension, entry->file_extension, 3);

        entry_name[8] = '\0';
        extension[3] = '\0';

        _strcpy(filename, entry_name);
        _strcat(filename, ".");
        _strcat(filename, extension);

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

        // found an entry
        dirent *dir_entry = &entries[i];
        _strcpy(dir_entry->d_name, filename);

        // for now: ignore directories
        if (entry->attributes & FAT_DIR_ENTRY_ATTR_DIRECTORY) {
            dir_entry->d_type = DT_DIR;
        }
        else {
            dir_entry->d_type = DT_REG;
        }

        // do not increment in for(..) because we need to skip some entries
        i++;
    }

    return i;
}

static ssize_t _fat32_fs_dir_file_seek(
    file_t *file, int32_t offset, int32_t whence)
{
    (void)file;
    (void)offset;
    (void)whence;
    kernel_fatal_error(__FUNCTION__);
    return -1;
}

static const file_ops_t _fat32_fs_dir_file_ops = {
    .read = NULL,
    .write = NULL,
    .seek = _fat32_fs_dir_file_seek,
    .readdir = _fat32_fs_dir_readdir,
};

//
// FAT32 Inode operations

static inode_t *_fat32fs_inode_lookup(inode_t *dir, const char *name)
{
    super_block_t *sb = dir->super_block;
    fat32_sb_private_t *sb_private = (fat32_sb_private_t*)sb->private;
    block_device_t *device = sb_private->device;

    const uint32_t dir_sector_index = dir->ino; // ino convention
    char sector[FAT32_SECTOR_SIZE];
    const int block_count = device->ops->read_block(device->private, dir_sector_index, sector);
    KERNEL_ASSERT(block_count == 1);

    fat_directory_entry_t *entry = (fat_directory_entry_t*)sector;
    for (; entry->filename[0] != 0x0u; entry++) {
        if (entry->filename[0] == 0xE5u) {
            // skip deleted file
            continue;
        }

        // retrieve entry name as a C string
        char entry_name[9];
        char extension[4];
        char filename[9 + 4 + 1];

        _memcpy(entry_name, entry->filename, 8);
        _memcpy(extension, entry->file_extension, 3);

        entry_name[8] = '\0';
        extension[3] = '\0';

        _strcpy(filename, entry_name);
        _strcat(filename, ".");
        _strcat(filename, extension);

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

        // check if name matche
        if (0 != _strcmp(filename, name)) {
            continue;
        }

        // found an entry

        // for now: ignore directories
        if (entry->attributes & FAT_DIR_ENTRY_ATTR_DIRECTORY) {
        }

        // entry is a file
        const uint32_t first_file_cluster = (
            (uint32_t)entry->starting_cluster_high << 16) | (entry->starting_cluster_low);
        const uint32_t first_file_sector = _fat32_cluster_sector_index(
            sb_private, first_file_cluster);

        // do we need to call read inode here ???????
        const ino_t ino = first_file_sector;
        inode_t *inode = sb->ops->alloc_inode(sb);

        inode->device = 0u;
        inode->file_ops = &_fat32_fs_dir_file_ops;
        inode->ino = ino;
        inode->inode_ops = NULL; // no inode ops for a regular file
        inode->link_count = 0u;
        inode->mode = S_IFREG;
        inode->private = NULL; // memory_calloc(sizeof(ramfs_dir_private_t));  
        inode->size = entry->file_size;

        return inode;
    }

    return NULL;
}

static const inode_ops_t _fat32fs_inode_ops = {
    .lookup = _fat32fs_inode_lookup,
    .create = NULL,
    .mkdir = NULL,
    .mknod = NULL,
    .link = NULL,
    .unlink = NULL,
    .rmdir = NULL
};

//
// FAT32 superblock operations

static inode_t *_fat32_sb_alloc_inode(super_block_t *fat32_sb)
{
    mini_uart_kernel_log("fat32fs: super-block: alloc inode ");
    inode_t *inode = memory_calloc(sizeof(inode_t));

    KERNEL_ASSERT(inode != NULL);
    inode->super_block = fat32_sb;

    return inode;
}

static void _fat32_sb_free_inode(super_block_t *fat32_sb, inode_t *inode)
{
    (void)inode;
    (void)fat32_sb;
    kernel_fatal_error("fat32fs: free inode: not implemented");
}

static int _fat32_sb_read_inode(super_block_t *fat32_sb, ino_t ino, inode_t *inode)
{
    // TODO: 
    // - in theory this should only be called on sb mount, but what if it
    //   is called another time ? 
    mini_uart_kernel_log("fat32: super-block: read inode %u", ino);

    // this is the only one which should be read one time on the fat32fs
    //
    // TODO: determine if something other than root node can be read
    KERNEL_ASSERT(ino == fat32_sb->root_ino); // use sector index as ino

    // should have been set by alloc
    KERNEL_ASSERT(inode->super_block == fat32_sb);

    // Load root node
    inode->device = 0u;
    inode->file_ops = &_fat32_fs_dir_file_ops;
    inode->ino = fat32_sb->root_ino;
    inode->inode_ops = &_fat32fs_inode_ops;
    inode->link_count = 0u;
    inode->mode = S_IFDIR;
    inode->private = NULL; // memory_calloc(sizeof(ramfs_dir_private_t));  
    inode->size = 0;

    return 0;
}

static int _fat32_sb_write_inode(super_block_t *fat32_sb, inode_t *inode)
{
    (void)fat32_sb;
    (void)inode;
    kernel_fatal_error("_fat32_sb_write_inode is not implemented");
    return -1;
}

static const super_block_ops_t _fat32fs_sb_ops = {
    .alloc_inode = _fat32_sb_alloc_inode ,
    .free_inode = _fat32_sb_free_inode ,
    .read_inode = _fat32_sb_read_inode ,
    .write_inode = _fat32_sb_write_inode 
};

//
// Public FAT32 filesystem API
// 
super_block_t *fat32_create_filesystem(block_device_t *blk_dev)
{
    mini_uart_kernel_log("fat32fs: create superblock");

    // allocate superblock
    super_block_t *sb = (super_block_t*)memory_calloc(sizeof(super_block_t));
    if (sb == NULL)
        return NULL;

    // allocate private data
    fat32_sb_private_t* private = _fat32_init_fs_private(blk_dev);
    KERNEL_ASSERT(private != NULL);

    // return the fat 32 superblock
    sb->ops = &_fat32fs_sb_ops;
    sb->private = private;
    sb->root_ino = private->root_cluster_sector_index;

    return sb;
}
