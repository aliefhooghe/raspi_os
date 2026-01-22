#include "hardware/mini_uart.h"
#include "kernel.h"
#include "kernel_types.h"
#include "lib/str.h"
#include "memory/memory_allocator.h"
#include "utils.h"
#include "vfs/dev/block_device_file_ops.h"
#include "vfs/device_ops.h"
#include "vfs/inode.h"
#include "vfs/super_block.h"
#include "vfs/driver_registry.h"
#include "ramfs.h"

#define RAMFS_ROOT_NODE_ID (1u)

//
// Backend
//

#define RAMFS_MAX_INODE_COUNT (32u)


// ramfs
typedef struct {
    // inode number generator
    ino_t ino_gen;
    
    // root inode: NULL if never loaded
    // inode_t *root;
} ramfs_t;

// ramfs dir
#define RAMFS_INODE_MAX_CHILDREN_COUNT (8)
#define RAMFS_MAX_NAME_LEN (32)

typedef struct {
    inode_t *inode;
    char name[RAMFS_MAX_NAME_LEN];    
} ramfs_inode_child_t;

typedef struct {
    ramfs_inode_child_t children[RAMFS_INODE_MAX_CHILDREN_COUNT];
    size_t child_count;
} ramfs_dir_private_t;

// ramfs file
#define RAMFS_INITIAL_FILE_ALLOC (512)

typedef struct {
    void *data;
    size_t alloc_size;
} ramfs_file_private_t;

//
// Ramfs Helpers
//
static void _add_inode_child(
    ramfs_dir_private_t *priv,
    const char *name,
    inode_t *inode)
{
    KERNEL_ASSERT(priv->child_count < RAMFS_INODE_MAX_CHILDREN_COUNT);

    const size_t child_index = priv->child_count++;
    priv->children[child_index].inode = inode;
    _strcpy(priv->children[child_index].name, name);
}

//
// VFS Interface Implementation
// 
static off_t _off_t_min(off_t a, off_t b)
{
    return a <= b ? a : b;
}

// static off_t _off_t_max(off_t a, off_t b)
// {
//     return a >= b ? a : b;
// }

static size_t _size_t_max(size_t a, size_t b)
{
    return a >= b ? a : b;
}

//
//
// Regular files operations
static ssize_t _ramfs_reg_file_read(
    file_t *file, void *data, size_t size)
{
    mini_uart_kernel_log("ramfs: reg_file_ops: read");
    KERNEL_ASSERT(file->inode != NULL);

    const off_t start = file->pos;
    if (start < 0)
        return -1;

    const off_t req_end = start + size;
    const off_t act_end = _off_t_min(req_end, file->inode->size);
    if (start >= act_end)
        return -1;

    const size_t act_size = act_end - start;
    ramfs_file_private_t *ramfs_file = (ramfs_file_private_t*)file->inode->private;

    // read data
    _memcpy(
        data,
        start + (uint8_t*)ramfs_file->data,
        act_size);
    file->pos = act_end;
    
    return act_size;
}

static ssize_t _ramfs_reg_file_write(
    file_t *file, const void *data, size_t size)
{
    mini_uart_kernel_log("ramfs: reg_file_ops: write");
    const off_t start = file->pos;
    if (start < 0)
        return -1;

    // no sparse file
    if (start > file->inode->size)
        return -1;

    const size_t end = start + size;
    const size_t new_size = _size_t_max(end, file->inode->size);
    ramfs_file_private_t *ramfs_file = (ramfs_file_private_t*)file->inode->private;

    // compute new alloc size
    size_t alloc_size = ramfs_file->alloc_size;
    while (alloc_size < new_size)
        alloc_size *= 2;

    // realloc if needed
    if (alloc_size > ramfs_file->alloc_size) {
        ramfs_file->data = memory_realloc(ramfs_file->data, alloc_size);
        KERNEL_ASSERT(ramfs_file->data != NULL);
        ramfs_file->alloc_size = alloc_size;
    }

    // write data
    _memcpy(
        start + (uint8_t*)ramfs_file->data,
        data, size);
    file->pos = end;
    file->inode->size = new_size;

    return size;
}

static ssize_t _ramfs_reg_file_seek(
    file_t *file, int32_t offset, int32_t whence)
{
    mini_uart_kernel_log("ramfs: reg_file_ops: seek");
    // compute reference
    off_t reference;
    switch (whence) {
        case SEEK_SET: reference = 0u; break;
        case SEEK_END: reference = file->inode->size; break;
        case SEEK_CUR: reference = file->pos; break;
        default:
            mini_uart_kernel_log(
                "ramfs: reg_file_seek: invalid whence: %u",
                whence);
            return -1;
    }

    const off_t new_pos = reference + offset;
    if (new_pos < 0)
        return -1;

    file->pos = new_pos;
    return file->pos;
}

static const file_ops_t _ramfs_reg_file_ops = {
    .read = _ramfs_reg_file_read,
    .write = _ramfs_reg_file_write,
    .seek = _ramfs_reg_file_seek,
    .readdir = NULL
};

//
// Directory file operations
static int _ramfs_dir_readdir(
    file_t *file, struct dirent *entries, size_t count)
{
    mini_uart_kernel_log("ramfs: dir_file_ops: readdir");
    inode_t *dir = file->inode;
    KERNEL_ASSERT(dir != NULL);

    ramfs_dir_private_t *priv_dir = (ramfs_dir_private_t*)dir->private;
    KERNEL_ASSERT(priv_dir != NULL);

    size_t i = 0u;
    for (;
        i < count && file->pos < (off_t)priv_dir->child_count;
        i++, file->pos++)
    {
        const ramfs_inode_child_t *child = &priv_dir->children[file->pos];
        dirent *entry = &entries[i];

        _strcpy(entry->d_name, child->name);

        // TODO: add a mapper somewere else name, inode => dirent
        switch (S_IFMT & child->inode->mode) {
            case S_IFDIR: entry->d_type = DT_DIR; break;
            case S_IFCHR: entry->d_type = DT_CHR; break;
            case S_IFREG: entry->d_type = DT_REG; break;
            default:
                kernel_fatal_error("ramfs: readdir: unhandled mode");
        }
    }

    return i;
}

static ssize_t _ramfs_dir_file_seek(
    file_t *file, int32_t offset, int32_t whence)
{
    mini_uart_kernel_log("ramfs: dir_file_ops: seek");
    inode_t *dir = file->inode;
    KERNEL_ASSERT(dir != NULL);

    ramfs_dir_private_t *priv_dir = (ramfs_dir_private_t*)dir->private;
    KERNEL_ASSERT(priv_dir != NULL);

    // compute reference
    off_t reference;
    switch (whence) {
        case SEEK_SET: reference = 0u; break;
        case SEEK_END: reference = priv_dir->child_count; break;
        case SEEK_CUR: reference = file->pos; break;
        default:
            mini_uart_kernel_log(
                "ramfs: dir_seek: invalid whence: %u",
                whence);
            return -1;
    }

    // compute new offset
    const off_t new_pos = reference + offset;

    // negative values is an error
    if (new_pos < 0)
        return -1;
    
    file->pos = new_pos;
    return file->pos;
}

static const file_ops_t _ramfs_dir_file_ops = {
    .read = NULL,
    .write = NULL,
    .seek = _ramfs_dir_file_seek,
    .readdir = _ramfs_dir_readdir,
};

//
// INode Operations
static inode_t *_ramfs_inode_lookup(inode_t *dir, const char *name)
{
    mini_uart_kernel_log(
        "ramfs: inode_ops: lookup: name='%s'",
        name);

    ramfs_dir_private_t *priv_dir = (ramfs_dir_private_t*)dir->private;

    for (size_t i = 0u; i < priv_dir->child_count; i++)
    {
        const ramfs_inode_child_t *child = &priv_dir->children[i];
        if (_strcmp(child->name, name) == 0) {
            KERNEL_ASSERT(child->inode != NULL);
            return child->inode;
        }
    }

    return NULL;
}

static inode_t *_ramfs_inode_create(inode_t *dir, const char *name, mode_t mode)
{
    mini_uart_kernel_log(
        "ramfs: inode_ops: create: name='%s' mode=0x%x",
        name, mode);

    if ((mode & S_IFMT) != S_IFREG) {
        mini_uart_kernel_log("ramfs: inode_ops: create: only regular file are supported");
        return NULL;
    }
    ramfs_t *ramfs = (ramfs_t*)dir->super_block->private;
    ramfs_dir_private_t *priv_dir = (ramfs_dir_private_t*)dir->private;
    inode_t *inode = dir->super_block->ops->alloc_inode(dir->super_block);

    // Allocate memory for file data
    ramfs_file_private_t *inode_file = (ramfs_file_private_t*)memory_calloc(sizeof(ramfs_file_private_t));
    KERNEL_ASSERT(inode_file != NULL); 
    inode_file->data = memory_calloc(RAMFS_INITIAL_FILE_ALLOC);
    KERNEL_ASSERT(inode_file->data != NULL);
    inode_file->alloc_size = RAMFS_INITIAL_FILE_ALLOC;
 
    inode->device = 0;
    inode->inode_ops = NULL; // no inode ops for a regular file
    inode->file_ops = &_ramfs_reg_file_ops;
    inode->private = (void*)inode_file;
    inode->size = 0u; // the file is created empty
    inode->ino = ++ramfs->ino_gen;
    inode->link_count = 0u;
    inode->mode = mode;

    _add_inode_child(priv_dir, name, inode);
    return inode;
}

static inode_t *_ramfs_inode_mkdir(inode_t *dir, const char *name, mode_t mode)
{
    // TODO:
    // - what if the dir already exists ?
    // - on which layer are done the check ?
    // 
    mini_uart_kernel_log(
        "ramfs: inode_ops: mkdir: name='%s' mode=0x%x",
        name, mode);

    if ((mode & S_IFMT) != S_IFDIR) {
        mini_uart_kernel_log("ramfs: mkdir: only directory are supported");
        return NULL;
    }

    if (NULL != _ramfs_inode_lookup(dir, name)) {
        mini_uart_kernel_log("ramfs: mkdir: name conflict %s", name);
        return NULL;
    }

    ramfs_t *ramfs = (ramfs_t*)dir->super_block->private;
    ramfs_dir_private_t *priv_dir = (ramfs_dir_private_t*)dir->private;
    inode_t *inode = dir->super_block->ops->alloc_inode(dir->super_block);

    inode->device = 0;
    inode->inode_ops = dir->inode_ops;
    inode->file_ops = &_ramfs_dir_file_ops;
    inode->size = 0u;
    inode->private = memory_calloc(sizeof(ramfs_dir_private_t));
    inode->ino = ++ramfs->ino_gen;
    inode->link_count = 0u;
    inode->mode = mode;

    _add_inode_child(priv_dir, name, inode);
    return inode;
}

static inode_t *_ramfs_inode_mknod(
    inode_t *dir, const char *name, mode_t mode, dev_t dev)
{
    mini_uart_kernel_log(
        "ramfs: inode_ops: mknod: name='%s' mode=0x%x dev=0x%x",
        name, mode, dev);

    // only chr device for now
    // TODO: block devices
    if ((mode & S_IFMT) != S_IFCHR) {
        mini_uart_kernel_log("ramfs: mknode: error: only character device are supported");
        return NULL;
    }

    const char_device_t *device = get_char_device(dev);
    if (device == NULL) {
        mini_uart_kernel_log(
            "ramfs: inode_ops: mknod: failed to open device");
        return NULL;
    }

    ramfs_t *ramfs = (ramfs_t*)dir->super_block->private;
    inode_t *inode = dir->super_block->ops->alloc_inode(dir->super_block);
    ramfs_dir_private_t *priv_dir = (ramfs_dir_private_t*)dir->private;

    inode->device = dev;
    inode->inode_ops = NULL;
    //  TODO: what to do with device.private ?
    inode->file_ops = device->ops;
    inode->private = NULL;
    inode->size = 0u; // ?
    inode->ino = ++ramfs->ino_gen;
    inode->link_count = 0u;
    inode->mode = mode;

    // inode ops does not make sense for a char device
    // it may make sense for special devices
    inode->inode_ops = NULL;

    _add_inode_child(priv_dir, name, inode);
    return inode;
}

static int _ramfs_inode_link(inode_t *src, inode_t *dir, const char *new_name)
{
    (void)src;
    (void)dir;
    (void)new_name;
    kernel_fatal_error("_ramfs_inode_link not implemented");
    return -1;
}

static int _ramfs_inode_unlink(inode_t *dir, const char *name)
{
    (void)dir;
    (void)name;
    kernel_fatal_error("_ramfs_inode_unlink not implemented");
    return -1;
}

static int _ramfs_inode_rmdir(inode_t *dir, const char *name)
{
    (void)dir;
    (void)name;
    kernel_fatal_error("_ramfs_inode_rmdir not implemented");
    return -1;
}

static const inode_ops_t _ramfs_inode_ops = {
    .lookup = _ramfs_inode_lookup,
    .create = _ramfs_inode_create,
    .mkdir = _ramfs_inode_mkdir,
    .mknod = _ramfs_inode_mknod,
    .link = _ramfs_inode_link,
    .unlink = _ramfs_inode_unlink,
    .rmdir = _ramfs_inode_rmdir
};

//
// Super Block Operations
static inode_t *_ramfs_sb_alloc_inode(super_block_t *ram_sb)
{
    mini_uart_kernel_log("ramfs: super-block: alloc inode ");
    inode_t *inode = memory_calloc(sizeof(inode_t));

    KERNEL_ASSERT(inode != NULL);
    inode->super_block = ram_sb;

    return inode;
}

static void _ramfs_sb_free_inode(super_block_t* sb, inode_t *inode)
{
    //
    //  TODO: what about childs ????
    // 
    (void)sb;
    KERNEL_ASSERT(inode!= NULL);
    KERNEL_ASSERT(inode->super_block == sb);
    if (inode->private != NULL)
        memory_free(inode->private);
    memory_free(inode);
}

static int _ramfs_sb_read_inode(super_block_t *ram_sb, ino_t ino, inode_t *inode)
{
    (void)ram_sb;

    // TODO: 
    // - in theory this should only be called on sb mount, but what if it
    //   is called another time ? 
    mini_uart_kernel_log("ramfs: super-block: read inode %u", ino);

    // this is the only one which should be read one time on the ramfs
    if (ino != RAMFS_ROOT_NODE_ID)
        return -1;

    // Load root node
    inode->device = 0u;
    inode->file_ops = &_ramfs_dir_file_ops;
    inode->ino = RAMFS_ROOT_NODE_ID;
    inode->inode_ops = &_ramfs_inode_ops;
    inode->link_count = 0u;
    inode->mode = S_IFDIR;
    inode->private = memory_calloc(sizeof(ramfs_dir_private_t));  
    inode->size = 0;

    return 0;
}

static int _ramfs_sb_write_inode(super_block_t *sb, inode_t *inode)
{
    (void)sb;
    (void)inode;
    kernel_fatal_error("_ramfs_sb_write_inode not implemented");
    return -1;
}

static const super_block_ops_t _ramfs_sb_ops = {
    .alloc_inode = _ramfs_sb_alloc_inode,
    .free_inode = _ramfs_sb_free_inode,
    .read_inode = _ramfs_sb_read_inode,
    .write_inode = _ramfs_sb_write_inode
};

//
// Public API
// 

super_block_t *create_ramfs_super_block(void)
{
    mini_uart_kernel_log("ramfs: create superblock");
    
    // allocate superblock
    super_block_t *sb = (super_block_t*)memory_calloc(sizeof(super_block_t));
    if (sb == NULL)
        return sb;
    sb->ops = &_ramfs_sb_ops;

    // allocate ramfs
    ramfs_t *ramfs = (ramfs_t*)memory_calloc(sizeof(ramfs_t));
    if (ramfs == NULL) {
        memory_free(sb);
        return NULL;
    }
    sb->private = ramfs;

    // Initialize root node id
    ramfs->ino_gen = RAMFS_ROOT_NODE_ID;
    sb->root_ino = RAMFS_ROOT_NODE_ID;

    return sb;
}
