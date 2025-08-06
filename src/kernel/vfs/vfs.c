
#include "hardware/mini_uart.h"

#include "kernel.h"
#include "kernel_types.h"
#include "lib/str.h"

#include "memory/bitfield.h"
#include "memory/section_allocator.h"

#include "vfs/filesystems/ramfs.h"
#include "vfs/super_block.h"

#include "vfs/vfs.h"
#include "vfs/dentry.h"
#include "vfs/inode.h"

//
// vfs
//
DEF_BITFIELD_ALLOCATOR(dentry_t, 72)
_Static_assert(
    BITFIELD_ALLOCATABLE_SIZE(dentry_t, 72) < 0x10000,
    "1/10 section reached");

typedef struct {
    dentry_t_x_72_bitfield_allocator_t dentry_allocator;
    dentry_t *root;
} vfs_t;

//
// global vfs
//
static vfs_t _vfs;

//
// Utils
//

static int _dentry_is_directory(dentry_t *dentry)
{
    // dentry lookup: does the inode exists ? 
    if (dentry == NULL || dentry->inode == NULL)
        return 0;

    // not null here.
    inode_t *dir_inode = dentry->inode;

    // is the inode a directory ?
    return (dir_inode->mode & S_IFMT) == S_IFDIR;
}

//
// Node lookup
// 
static dentry_t *_vfs_dentry_child_by_name(
    dentry_t *parent,  // non negative dentry
    const char *name)
{
    // direct dentry lookup
    for (size_t index = 0u; index < parent->child_count; index++)
    {
        dentry_t* child = parent->children[index];
        if (0 == _strcmp(name, child->name))
            return child;
    }

    // dentry cache miss: lookup in inode
    KERNEL_ASSERT(parent->inode->inode_ops != NULL &&
                  parent->inode->inode_ops->lookup != NULL);
    inode_t *child = parent->inode->inode_ops->lookup(parent->inode, name);
    dentry_t *new_dentry = dentry_t_x_72_bitfield_alloc(&_vfs.dentry_allocator);

    _strcpy(new_dentry->name, name);
    new_dentry->child_count = 0u;
    new_dentry->inode = child;  // if child is null, this is a negative dentry
    new_dentry->parent = parent;

    if (parent->child_count == DENTRY_MAX_CHILREN_COUNT)
        kernel_fatal_error("reached max dentry child cound");

    parent->children[parent->child_count++] = new_dentry;

    return new_dentry;
}

static dentry_t *_vfs_node_lookup_rec(
    dentry_t *dentry,  // non negative dentry
    const char *path)
{
    mini_uart_kernel_log(
        "vfs: inode lookup: look for relative path '%s' in dentry %s",
         path, dentry->name);
    const char *next_separator = _strchr(path, '/');
    if (next_separator == NULL) {
        // then path itself is the last segment
        const uint32_t name_len = _strlen(path);
        if (name_len == 0u)
             return dentry;
        return _vfs_dentry_child_by_name(dentry, path);
    }
    else {
        const uint32_t child_name_len = next_separator - path;

        // HOORRIIIIBLE but ok for now.
        static char name[DENTRY_MAX_NAME_LEN] = "";
        _memcpy(name, path, child_name_len);
        name[child_name_len] = '\0';

        dentry_t *child = _vfs_dentry_child_by_name(dentry, name);
        if (child->inode == NULL) {
            // we do not want to create child on negative dentry
            return NULL;  // no dentry exists now
        }
        else if ((child->inode->mode & S_IFMT) != S_IFDIR)
        {
            // not a dir: lookup is not possible
            return NULL;
        }

        return _vfs_node_lookup_rec(child, path + child_name_len + 1);
    }
}

//
// return:
// - NULL if parent path doesn't exists / is not a directory
// - a negative dentry if path does not exists but parent path is a dir
// - a positive dentry if path exists
static dentry_t *_vfs_dentry_lookup(const char *path)
{
    mini_uart_kernel_log("vfs: dentry lookup: search path '%s'", path);
    // only absolute path for now
    if (path[0] != '/')
        return NULL;
    dentry_t *result = _vfs_node_lookup_rec(_vfs.root, path + 1);

    // if a dentry is returned, parent inode exists and is a directory
    KERNEL_ASSERT(
        result == NULL ||           // no dentry
        result->inode != NULL ||    // path exist
        (                           // negative dentry: expect to have a parent
            result->parent != NULL &&
            result->parent->inode != NULL &&
            (result->parent->inode->mode & S_IFMT) == S_IFDIR));

    if (result == NULL)
        mini_uart_kernel_log("vfs: dentry lookup: path='%s': failed (no dentry)", path);
    else if (result->inode == NULL)
        mini_uart_kernel_log("vfs: dentry lookup: path='%s': failed (negative dentry)", path);
    else
        mini_uart_kernel_log("vfs: dentry lookup: path='%s': succeed", path);

    return result;
}

//
//  VFS: Initialization
//
void vfs_init(void)
{
    _memset(&_vfs, 0, sizeof(vfs_t));

    // allocate memory section for vfs
    _vfs.dentry_allocator.base = section_allocator_alloc();
    if (_vfs.dentry_allocator.base == NULL)
        kernel_fatal_error("vfs dentry memory section allocation failed");

    // Create a initial root inode: we need to have a '/' dir to mount on it
    static inode_t init_root_node;
    _memset(&init_root_node, 0, sizeof(inode_t));
    init_root_node.mode = S_IFDIR;

    // Insert a root dentry with the initial inode
    dentry_t *root_dentry = dentry_t_x_72_bitfield_alloc(&_vfs.dentry_allocator);
    root_dentry->inode = &init_root_node;
    root_dentry->child_count = 0u;
    root_dentry->parent = NULL;
    _strcpy(root_dentry->name, "root");
    
    _vfs.root = root_dentry;
}

//
//  VFS: Filesystem interface
//

static super_block_t *_sb_by_dev_fs_type(const char *dev, const char *fstype)
{
    (void)dev;
    (void)fstype;
    return create_ramfs_super_block();
}

int32_t vfs_mount(const char *dev, const char *target, const char *fstype)
{
    mini_uart_kernel_log(
        "vfs: mount: dev='%s', target='%s', fstype=%s",
        dev, target, fstype);

    // Load root inode from fs super block
    super_block_t *sb = _sb_by_dev_fs_type(dev, fstype);
    inode_t *root = sb->ops->alloc_inode(sb);
    KERNEL_ASSERT(root != NULL);
    const int load_status = sb->ops->read_inode(sb, sb->root_ino, root);
    KERNEL_ASSERT(load_status == 0);

    // Check if target is a directory
    dentry_t *target_dentry = _vfs_dentry_lookup(target);
    if (!_dentry_is_directory(target_dentry)) {
        mini_uart_kernel_log(
            "vfs: mount: no directory at '%s'", target);
        return -1;
    }

    // Mount the root inode on the target dentry
    target_dentry->inode = root;
    target_dentry->child_count = 0u;  // flush dentry child cache
    
    return 0;
}

int32_t vfs_mknod(const char *path, mode_t mode, dev_t dev)
{
    mini_uart_kernel_log(
        "vfs: mknod: path='%s', mode=0x%x, dev=0x%x",
        path, mode, dev);

    dentry_t *dentry = _vfs_dentry_lookup(path);
    if (dentry == NULL)
        return -1; // no parent dir
    else if (dentry->inode != NULL)
        return -1; // a file already exists at this path

    inode_t *parent = dentry->parent->inode;
    inode_t *new_node = parent->inode_ops->mknod(
        parent, dentry->name, mode, dev);

    if (new_node == NULL)
        return -1;

    // update dentry
    dentry->inode = new_node;
    return 0;
}

int32_t vfs_mkdir(const char *path, mode_t mode)
{
    mini_uart_kernel_log(
        "vfs: mknod: path='%s', mode=0x%x",
        path, mode);

    dentry_t *dentry = _vfs_dentry_lookup(path);
    if (dentry == NULL)
        return -1; // no parent dir
    else if (dentry->inode != NULL)
        return -1; // a file already exists at this path

    inode_t *parent = dentry->parent->inode;
    inode_t *new_node = parent->inode_ops->mkdir(
        parent, dentry->name, mode);

    if (new_node == NULL)
        return -1;

    // update dentry
    dentry->inode = new_node;
    return 0;
}

//
//  VFS: File interface
//

file_t *vfs_file_open(const char *path, uint32_t flags, mode_t mode)
{
    (void)mode;

    mini_uart_kernel_log("vfs: open %s", path);
    dentry_t *dentry = _vfs_dentry_lookup(path);

    if (dentry == NULL)
        return NULL; // no parent dir

    inode_t *parent = dentry->parent->inode;
    
    if (dentry->inode == NULL)
    {
        if ((flags & O_CREAT) &&
            !(flags & O_DIRECTORY)) // can't create a directory with open
        { 
            // EXCL does not mater here.
            inode_t *new_inode = parent->inode_ops->create(
                parent, dentry->name, mode);
            if (new_inode == NULL)
                return NULL; // creation failed

            // update dentry
            dentry->inode = new_inode;
        }
        else
        {
            return NULL;  // file not found, no creation requested
        }
    }
    else if (flags & O_EXCL)
    {
        return NULL;  // file already exists
    }
    
    // wether created or existing: inode should exists
    inode_t *inode = dentry->inode;
    KERNEL_ASSERT(inode != NULL);
    
    // there is an existing inode
    if ((flags & O_DIRECTORY) && ((inode->mode & S_IFMT) != S_IFDIR)) {
        return NULL;
    }
    else if(inode->file_ops == NULL ||
       inode->file_ops->open == NULL)
    {
        mini_uart_kernel_log(
            "vfs: open: inode at '%s' does not implement open file operation",
            path);
        return NULL;
    }

    mini_uart_kernel_log("vfs: call backend");
    return inode->file_ops->open(inode);
}

int32_t vfs_file_close(file_t *file)
{
    if (file->inode->file_ops == NULL ||
        file->inode->file_ops->release == NULL)
        return -1;
    return file->inode->file_ops->release(file->inode, file);
}

ssize_t vfs_file_read(file_t *file, void *data, size_t size)
{
    if (file->inode->file_ops == NULL ||
        file->inode->file_ops->read == NULL)
        return -1;
    return file->inode->file_ops->read(file, data, size, &file->pos);
}

ssize_t vfs_file_readdir(file_t *file, dirent *entries, size_t count)
{
    if (file->inode->file_ops == NULL ||
        file->inode->file_ops->readdir == NULL)
        return -1;
    return file->inode->file_ops->readdir(file, entries, count);
}

ssize_t vfs_file_write(file_t *file, const void *data, size_t size)
{
    if (file->inode->file_ops == NULL ||
        file->inode->file_ops->write == NULL)
        return -1;
    return file->inode->file_ops->write(file, data, size, &file->pos);
}

ssize_t vfs_file_lseek(file_t *file, int32_t offset, int32_t whence)
{
    if (file->inode->file_ops == NULL ||
        file->inode->file_ops->seek == NULL)
        return -1;
    return file->inode->file_ops->seek(file, offset, whence);
}
