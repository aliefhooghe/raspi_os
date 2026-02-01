
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
// We are using the directory entry offset to encode the inode.
// root does not have a directory entry so we set root ino = 2
// 
#define FAT32_SECTOR_SIZE 512u       // we only handle 512 sectors
#define FAT32_ROOT_INO    2u

// locate a fat directory entry in the disk
typedef struct {
    uint32_t entry_index;    // entry index in sector (0-15 for 512b sector)
    uint32_t sector_index;   // sector index in the cluster
    uint32_t cluster;        // cluster in the disk
} fat32_dir_entry_loc_t;

// fat32 super block private data
typedef struct {
    block_device_t *device;                // underlying block device

    uint32_t data_sector_index;            // data area sector index
    uint32_t sectors_per_cluster;          // nb of sector per cluster. Sector size is device->block_size
    uint32_t root_cluster;                 // root cluster number
} fat32_sb_private_t;

// fat 32 inode private data is content cluster num

// fat 32 regular file private data
typedef struct {
    uint32_t offset;
    uint32_t sector_index;
    uint32_t cluster;
    size_t file_size;
} fat32_file_reg_private_t;

// fat32 directory file private data
typedef fat32_dir_entry_loc_t fat32_file_dir_private_t;

//
// FAT32 Helpers
//
static int _is_fat32_boot_sector(const fat_boot_sector_t *bs)
{
    KERNEL_ASSERT(bs->bytes_per_sector == FAT32_SECTOR_SIZE);
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

// load a fat32 filesystem
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
    private->root_cluster = ebs->root_cluster;

    return private;
}

// compute directory entry filename (short)
static void _fat32_read_entry_filename(
    const fat_directory_entry_t *entry,
    char filename[16])
{
    int fi = 0u;
    for (int i = 0; i < 8 && entry->filename[i] != ' '; i++, fi++) {
        filename[fi] = entry->filename[i];
    }
    if (0 == (entry->attributes & FAT_DIR_ENTRY_ATTR_DIRECTORY))
    {
        filename[fi++] = '.';
        filename[fi++] = entry->file_extension[0];
        filename[fi++] = entry->file_extension[1];
        filename[fi++] = entry->file_extension[2];
    }
    filename[fi++] = '\0';
}

// return the inode CONTENT cluster number
static uint32_t _fat32_get_inode_cluster(const inode_t *inode)
{
    // inode private is the cluster number.
    // This is not data duplication: we initialize the CURRENT cluster
    // which may move through call to readdir / seek
    return (uint32_t)inode->private;
}

// create an inode abstraction on fat directory entry
static inode_t *_fat32_create_inode(
    const fat_directory_entry_t *entry,
    super_block_t *sb,
    ino_t ino);

// entry loc utils
static uint32_t _fat32_entry_loc_sector(
    fat32_sb_private_t *sb_private,
    const fat32_dir_entry_loc_t *entry_loc)
{
    return
        _fat32_cluster_sector_index(sb_private, entry_loc->cluster) +
        entry_loc->sector_index;
}

static uint32_t _fat32_entry_loc_offset(
    fat32_sb_private_t *sb_private,
    const fat32_dir_entry_loc_t *entry_loc)
{
    const uint32_t entry_sector = _fat32_entry_loc_sector(sb_private, entry_loc);
    return
        (entry_sector * FAT32_SECTOR_SIZE) +
        entry_loc->entry_index * sizeof(fat_directory_entry_t);
}

// directory entry iterator helper:
typedef struct {
    int cache_is_dirty;
    fat32_dir_entry_loc_t entry_loc;
    uint8_t sector_cache[FAT32_SECTOR_SIZE];
} fat_directory_entry_iterator_t;

static void fat_32_directory_entry_iterator_init(
    fat_directory_entry_iterator_t *it,
    const fat32_dir_entry_loc_t *entry_loc)
{
    it->cache_is_dirty = 1;
    it->entry_loc = *entry_loc;
}

static const fat_directory_entry_t *fat_32_directory_entry_iterator_next(
    fat_directory_entry_iterator_t *it,
    fat32_sb_private_t *sb_private,
    ino_t *entry_ino)
{
    block_device_t *device = sb_private->device;

    for (;;)
    {
        // load from device to cache if required
        if (it->cache_is_dirty) {
            // compute current sector index
            const uint32_t sector = _fat32_entry_loc_sector(
                sb_private, &it->entry_loc);
            // load sector
            mini_uart_kernel_log("fat32: block device: load sector n°%u", sector);
            const int block_count = device->ops->read_block(
                device->private, sector, it->sector_cache);
            KERNEL_ASSERT(block_count == 1);
            it->cache_is_dirty = 0;
        }

        // retrieve entry from sector (cache)
        mini_uart_kernel_log("fat32: retrieve entry @ cluster=%u sector=%u index=%u",
            it->entry_loc.cluster,
            it->entry_loc.sector_index,
            it->entry_loc.entry_index);
        const fat_directory_entry_t *entry =
            it->entry_loc.entry_index + (fat_directory_entry_t*)it->sector_cache;

        // reached end
        if (entry->filename[0] == 0x0u)
        {
            mini_uart_kernel_log("fat32: entry is END");
            return NULL;
        }

        // compute directory entry offset
        const uint32_t entry_offset =
            _fat32_entry_loc_offset(sb_private, &it->entry_loc);

        // index goto next entry
        it->entry_loc.entry_index++;
        if (it->entry_loc.entry_index >= 16) {
            // reached the end of sector
            it->entry_loc.sector_index++;
            it->entry_loc.entry_index = 0u;
            it->cache_is_dirty = 1;

            // do not handle 
            if (it->entry_loc.sector_index >= sb_private->sectors_per_cluster)
            {
                mini_uart_kernel_log("fat32: sector per cluster = %u", sb_private->sectors_per_cluster);
                kernel_fatal_error("fat32: reached end of cluster");
            }
        }

        // skip unused entries
        if (
            (entry->filename[0] == 0xE5u) ||                   // skip deleted file
            (entry->attributes & FAT_DIR_ENTRY_ATTR_LFN) ||    // skip long file name entries
            (entry->attributes & FAT_DIR_ENTRY_ATTR_VOLUME_ID) // skip volume id
        ) {
            mini_uart_kernel_log("fat32: skip entry");
            continue;
        }

        mini_uart_kernel_log("fat32: keep entry");
        *entry_ino = entry_offset;
        return entry;
    }
}

//
// FAT32 VFS interface implementation
//

//
// FAT32 Regular file operations

static file_t *_fat32fs_reg_file_open(inode_t *inode)
{
    mini_uart_kernel_log("fat32_fs: reg_file_ops: open");
    fat32_file_reg_private_t *reg_private =
        memory_calloc(sizeof(fat32_file_reg_private_t ));

    if (reg_private == NULL) {
        return NULL;
    }

    file_t *file = default_file_open(inode);
    if (file == NULL) {
        memory_free(reg_private);
        return NULL;
    }

    reg_private->cluster = _fat32_get_inode_cluster(inode);
    file->private = reg_private;
    return file;
}

static int _fat32fs_reg_file_release(inode_t *inode, file_t *file)
{
    memory_free(file->private);
    return default_file_release(inode, file);
}

static ssize_t _fat32fs_reg_file_read(
    file_t *file, void *data, size_t size)
{
    (void)file;
    (void)data;
    (void)size;
    kernel_fatal_error(__FUNCTION__);
    return -1;
}

static ssize_t _fat32fs_reg_file_write(
    file_t *file, const void *data, size_t size)
{
    (void)file;
    (void)data;
    (void)size;
    kernel_fatal_error(__FUNCTION__);
    return -1;
}

static ssize_t _fat32fs_reg_file_seek(
    file_t *file, int32_t offset, int32_t whence)
{
    (void)file;
    (void)offset;
    (void)whence;
    kernel_fatal_error(__FUNCTION__);
    return -1;
}

static const file_ops_t _fat32_fs_reg_file_ops = {
    .open = _fat32fs_reg_file_open,
    .release = _fat32fs_reg_file_release,
    .read = _fat32fs_reg_file_read,
    .write = _fat32fs_reg_file_write,
    .seek = _fat32fs_reg_file_seek,
    .readdir = NULL
};

//
// FAT32 Directory file operations

static file_t *_fat32_fs_dir_open(inode_t *inode)
{
    mini_uart_kernel_log("fat32_fs: dir_file_ops: open");
    fat32_file_dir_private_t *dir_private =
        memory_calloc(sizeof(fat32_file_dir_private_t));

    if (dir_private == NULL) {
        return NULL;
    }

    file_t *file = default_file_open(inode);
    if (file == NULL) {
        memory_free(dir_private);
        return NULL;
    }

    dir_private->cluster = _fat32_get_inode_cluster(inode);
    file->private = dir_private;
    return file;
}

static int _fat32_fs_dir_release(inode_t *inode, file_t *file)
{
    memory_free(file->private);
    return default_file_release(inode, file);
}

static int _fat32_fs_dir_readdir(
    file_t *file, struct dirent *entries, size_t count)
{
    mini_uart_kernel_log("fat32fs: dir_file_ops: readdir");

    inode_t *dir = file->inode;
    KERNEL_ASSERT(dir != NULL);

    super_block_t *sb = dir->super_block;
    fat32_sb_private_t *sb_private = (fat32_sb_private_t*)sb->private;
    KERNEL_ASSERT(file->private != NULL);

    const fat_directory_entry_t *entry = NULL;
    ino_t entry_ino = 0u;
    fat_directory_entry_iterator_t it;

    // start at recorded entry location
    fat32_file_dir_private_t *entry_loc = (fat32_file_dir_private_t*)file->private;
    fat_32_directory_entry_iterator_init(&it, entry_loc);

    size_t i = 0u; // dirent counter
    while (
        (i < count) &&
        (NULL != (entry = fat_32_directory_entry_iterator_next(&it, sb_private, &entry_ino))))
    {
        dirent *dir_entry = &entries[i];
        _fat32_read_entry_filename(entry, dir_entry->d_name);
        dir_entry->d_type =
            (entry->attributes & FAT_DIR_ENTRY_ATTR_DIRECTORY) ?
            DT_DIR : DT_REG;

        i++;
        file->pos++;
    }

    // write back entry location to file private data
    *entry_loc = it.entry_loc;
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
    .open = _fat32_fs_dir_open,
    .release = _fat32_fs_dir_release,
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

    const fat_directory_entry_t *entry = NULL;
    ino_t entry_ino = 0u;
    fat_directory_entry_iterator_t it;

    // start at first cluster entry
    const fat32_dir_entry_loc_t entry_loc = {
        .cluster = _fat32_get_inode_cluster(dir),
        .sector_index = 0u,
        .entry_index = 0u
    };
    fat_32_directory_entry_iterator_init(&it, &entry_loc);

    while (NULL != (entry = fat_32_directory_entry_iterator_next(&it, sb_private, &entry_ino)))
    {
        char filename[16];
        _fat32_read_entry_filename(entry, filename);

        // check if name matche
        if (0 != _strcmp(filename, name))
        {
            continue;
        }

        // do we need to call read inode here ???????
        return _fat32_create_inode(entry, sb, entry_ino);
    }

    return NULL;
}

static const inode_ops_t _fat32_fs_inode_ops = {
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
    // TODO: default impl like for fileops.open ? Decide why private is not allocated here
    mini_uart_kernel_log("fat32fs: super-block: alloc inode ");
    inode_t *inode = memory_calloc(sizeof(inode_t));

    KERNEL_ASSERT(inode != NULL);
    inode->super_block = fat32_sb;

    return inode;
}

static void _fat32_sb_free_inode(super_block_t *fat32_sb, inode_t *inode)
{
    (void)fat32_sb;
    memory_free(inode);
}

static int _fat32_sb_read_inode(super_block_t *fat32_sb, ino_t ino, inode_t *inode)
{
    fat32_sb_private_t *sb_private = (fat32_sb_private_t*)fat32_sb->private;

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
    inode->inode_ops = &_fat32_fs_inode_ops;
    inode->link_count = 0u;
    inode->mode = S_IFDIR; // root is a dir

    // inode private is the content cluster
    inode->private = (void*)sb_private->root_cluster;
    inode->size = 0; // 0 for a dir. TODO ???

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
// Fat32 inode creation

static inode_t *_fat32_create_inode(
    const fat_directory_entry_t *entry,
    super_block_t *sb,
    ino_t ino)
{
    const int is_dir = (entry->attributes & FAT_DIR_ENTRY_ATTR_DIRECTORY);
    const uint32_t cluster_num = (
        entry->starting_cluster_low | (entry->starting_cluster_high << 16)
    );

    mini_uart_kernel_log(
        "fat32: create inode: ino=%u cluster num=%u mode=%s",
        ino, cluster_num, is_dir ? "DIR" : "REG");

    // create inode
    inode_t *inode = sb->ops->alloc_inode(sb);

    inode->device = 0u;
    inode->file_ops = is_dir ? &_fat32_fs_dir_file_ops : &_fat32_fs_reg_file_ops;
    inode->ino = ino;
    inode->inode_ops = is_dir ? &_fat32_fs_inode_ops : NULL;
    inode->link_count = 0u;
    inode->mode = is_dir ? S_IFDIR : S_IFREG;
    inode->private = (void*)cluster_num;
    inode->size = entry->file_size;

    return inode;
}

//
// Public FAT32 filesystem API
// 
super_block_t *fat32_create_filesystem(block_device_t *blk_dev)
{
    mini_uart_kernel_log("fat32fs: create superblock");

    // allocate private data
    fat32_sb_private_t* private = _fat32_init_fs_private(blk_dev);
    if (private == NULL) {
        mini_uart_kernel_log("fat32fs: failed to load a fat32 fs from device");
        return NULL;
    }

    // allocate superblock
    super_block_t *sb = (super_block_t*)memory_calloc(sizeof(super_block_t));
    if (sb == NULL)
        return NULL;

    // return the fat 32 superblock
    sb->ops = &_fat32fs_sb_ops;
    sb->private = private;
    sb->root_ino = FAT32_ROOT_INO;

    return sb;
}
