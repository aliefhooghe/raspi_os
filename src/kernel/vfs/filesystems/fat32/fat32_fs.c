
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
// TODO: this will break whith empty files ! Lets use (sector << 4  | index(0-15))
// 
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
    // Load a fat32 filesystem from a block device
    // 
    uint8_t sector[FAT32_SECTOR_SIZE];

    // only handle 512 bytes sectors
    KERNEL_ASSERT(device->block_size == FAT32_SECTOR_SIZE);

    // 
    // read the boot sector
    const int status = device->ops->read_block(device->private, 0u, sector);
    KERNEL_ASSERT(status == 1u);

    // validate boot sector
    const fat_boot_sector_t *boot_sector = (fat_boot_sector_t*)sector;
    if (!_is_fat32_boot_sector(boot_sector)) {
        mini_uart_kernel_log("fat32fs: not a fat32 filesystem");
        return NULL;
    }

    const size_t sector_count = (
        boot_sector->total_sectors_32 == 0 ?
            boot_sector->total_sectors_16 :
            boot_sector->total_sectors_32);

    mini_uart_kernel_log("[FAT32] sector count = %u", sector_count);

    //
    // read fat32 extended boot sector
    fat_extended_boot_sector32_t *ebs = (fat_extended_boot_sector32_t*)(&boot_sector[1]);

    // compute data area sector index (first data sector)
    const uint32_t data_aera_sector_index =
        boot_sector->reserved_sector_count +
        boot_sector->fat_count * ebs->table_size_32;

    // allocate fs private data
    fat32_sb_private_t* private = (fat32_sb_private_t*)memory_calloc(sizeof(fat32_sb_private_t));
    if (private == NULL) {
        return NULL;
    }

    private->device = device; // keep a reference to block device
    private->data_sector_index = data_aera_sector_index;
    private->sectors_per_cluster = boot_sector->sectors_per_cluster;

    // compute root cluster location.
    private->root_cluster_sector_index =
        _fat32_cluster_sector_index(private, ebs->root_cluster);

    return private;
}

static void _fat32_read_entry_filename(
    const fat_directory_entry_t *entry,
    char filename[16])
{
    char entry_name[9];
    char extension[4];

    _memcpy(entry_name, entry->filename, 8);
    _memcpy(extension, entry->file_extension, 3);

    entry_name[8] = '\0';
    extension[3] = '\0';

    _strcpy(filename, entry_name);
    _strcat(filename, ".");
    _strcat(filename, extension);
}

static uint32_t _fat32_entry_start_sector(
    const fat_directory_entry_t *entry,
    const fat32_sb_private_t *sb_private)
{
    const uint32_t first_cluster = (
        (uint32_t)entry->starting_cluster_high << 16) | (entry->starting_cluster_low);
    return _fat32_cluster_sector_index(sb_private, first_cluster);
}

// directory entry iterator helper:

typedef struct {
    uint32_t entry_index;
    uint8_t current_sector[FAT32_SECTOR_SIZE];
} fat_directory_entry_iterator_t;

static void fat_32_directory_entry_iterator_init(
    fat_directory_entry_iterator_t *it,
    block_device_t *device,
    uint32_t sector_index,
    uint32_t entry_index)
{
    const int block_count = device->ops->read_block(
        device->private, sector_index, it->current_sector);
    KERNEL_ASSERT(block_count == 1);
    it->entry_index = entry_index;
}

static fat_directory_entry_t *fat_32_directory_entry_iterator_next(
    fat_directory_entry_iterator_t *it)
{
    fat_directory_entry_t *entry = it->entry_index + (fat_directory_entry_t*)it->current_sector;

    for (;
        entry->filename[0] != 0x0u;
        entry++, it->entry_index++)
    {
        if (
            (entry->filename[0] == 0xE5u) ||                   // skip deleted file
            (entry->attributes & FAT_DIR_ENTRY_ATTR_LFN) ||    // skip long file name entries
            (entry->attributes & FAT_DIR_ENTRY_ATTR_VOLUME_ID) // skip volume id
        ) {
            continue;
        }

        return entry;
    }

    return NULL;
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
    // TODO: actually a bug: we don't now how many skiped entr there is
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
        char filename[16];
        _fat32_read_entry_filename(entry, filename);

        // check for Long File Name entry first
        if (entry->attributes & FAT_DIR_ENTRY_ATTR_LFN) {
            mini_uart_kernel_log("[FAT32] skip LFN entry: %s", filename);
            continue;
        }
        // check if the entry is a volume ID
        else if (entry->attributes & FAT_DIR_ENTRY_ATTR_VOLUME_ID) {
            mini_uart_kernel_log("[FAT32] volume id: %s", filename);
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
    .open = default_file_open,
    .release = default_file_release,
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
    fat_directory_entry_t *entry = NULL;
    fat_directory_entry_iterator_t it;

    // start from first entry
    fat_32_directory_entry_iterator_init(&it, device, dir_sector_index, 0u);

    while (NULL != (entry = fat_32_directory_entry_iterator_next(&it)))
    {
        char filename[16];
        _fat32_read_entry_filename(entry, filename);

        // check if name matche
        if (
            (0 != _strcmp(filename, name)) ||  // check if name matches
            (entry->attributes & FAT_DIR_ENTRY_ATTR_DIRECTORY)  // ignore directories TODO
        ) {
            continue;
        }

        const uint32_t start_sector = _fat32_entry_start_sector(entry, sb_private);

        // do we need to call read inode here ???????
        const ino_t ino = start_sector;
        inode_t *inode = sb->ops->alloc_inode(sb);

        inode->device = 0u;
        inode->file_ops = &_fat32_fs_dir_file_ops;
        inode->ino = ino;
        inode->inode_ops = NULL; // no inode ops for a regular file
        inode->link_count = 0u;
        inode->mode = S_IFREG;
        inode->private = NULL;
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
    inode->private = NULL;
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

    // allocate private data
    fat32_sb_private_t* private = _fat32_init_fs_private(blk_dev);
    if (private == NULL) {
        mini_uart_kernel_log("fat32fs: failed to load a fat32 fs from device\n");
        return NULL;
    }

    // allocate superblock
    super_block_t *sb = (super_block_t*)memory_calloc(sizeof(super_block_t));
    if (sb == NULL)
        return NULL;

    // return the fat 32 superblock
    sb->ops = &_fat32fs_sb_ops;
    sb->private = private;
    sb->root_ino = private->root_cluster_sector_index;

    return sb;
}
