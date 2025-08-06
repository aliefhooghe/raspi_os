
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

static int _basedir(const char *filepath, char *basedir, char *filename)
{
    mini_uart_kernel_log("vfs: basedir: filepath='%s'", filepath);
    
    // compute path len
    const size_t path_len = _strlen(filepath);
    if (path_len == 0u)
        return -1;

    const char *last_separator = _strrchr(filepath, '/');
    if (last_separator == NULL) {
        mini_uart_kernel_log("vfs: basedir: relavite path are not handled");
        return -1;
    }

    const size_t basedir_path_len = 1 + last_separator - filepath;
    // check last car: we do not want a directory path
    if (basedir_path_len == (path_len - 1))
        return -1;

    // retrieve base dir and filename
    _memcpy(basedir, filepath, basedir_path_len);
    basedir[basedir_path_len] = '\0';
    _strcpy(filename, last_separator + 1);

    mini_uart_kernel_log(
        "vfs: basedir: filepath='%s' => .basedir='%s' .filename='%s'",
         filepath, basedir, filename);
   
    return 0;
}

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
        if (child->inode == NULL)
            return NULL;  // no dentry

        return _vfs_node_lookup_rec(child, path + child_name_len + 1);
    }
}

static dentry_t *_vfs_dentry_lookup(const char *path)
{
    mini_uart_kernel_log("vfs: dentry lookup: search path '%s'", path);
    // only absolute path for now
    if (path[0] != '/')
        return NULL;
    dentry_t *result = _vfs_node_lookup_rec(_vfs.root, path + 1);

    if (result == NULL)
        mini_uart_kernel_log("vfs: dentry lookup: path='%s': failed (no dentry)", path);
    else if (result->inode == NULL)
        mini_uart_kernel_log("vfs: dentry lookup: path='%s': failed (negative dentry)", path);
    else
        mini_uart_kernel_log("vfs: dentry lookup: path='%s': succeed", path);

    return result;
}

static int _check_new_path(
    const char *path,
    char *basedir_path,
    dentry_t **basedir,
    dentry_t **child)
{
    char filename[DENTRY_MAX_NAME_LEN] = "";
    
    const int status = _basedir(path, basedir_path, filename);
    if (status != 0)
        return status;

    // dentry lookup: does the basedire exists ? 
    dentry_t* dir_dentry = _vfs_dentry_lookup(basedir_path);
    if (dir_dentry == NULL || dir_dentry->inode == NULL) {
        mini_uart_kernel_log(
            "vfs: mknod: base directory '%s' does not exists",
            basedir_path);
        return -1;
    }

    // inode is not null here.
    inode_t *dir_inode = dir_dentry->inode;

    // is the inode a directory ?
    if ((dir_inode->mode & S_IFMT) != S_IFDIR) {
        mini_uart_kernel_log(
            "vfs: check new path: base path '%s' is not a directory",
            basedir_path);
        return -1;
    }

    // check if the path already exists
    dentry_t *existing_dentry = _vfs_dentry_child_by_name(dir_dentry, filename);
    if (existing_dentry->inode != NULL) {
        mini_uart_kernel_log(
            "vfs: check new path: path '%s' already exists", path);
        return -1;
    }

    // Ok: parent exists and is a directory and the path is unused
    *basedir = dir_dentry;
    *child = existing_dentry;
    return 0;
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

    // Mount a ramfs on /
    const int32_t root_mount_status = vfs_mount("dev", "/", "fstype");
    KERNEL_ASSERT(0 == root_mount_status);
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

    // get the basedir path
    char basedir_path[256] = "";
    dentry_t *basedir = NULL;
    dentry_t *new_child = NULL;

    const int status = _check_new_path(path, basedir_path, &basedir, &new_child);
    if (status != 0)
        return status;

    KERNEL_ASSERT(basedir != NULL && basedir->inode != NULL);
    KERNEL_ASSERT(new_child != NULL && new_child->inode == NULL);

    // call fs backend.
    inode_t *basedir_inode = basedir->inode;
    if (basedir_inode->inode_ops == NULL ||
        basedir_inode->inode_ops->mknod == NULL) {
        mini_uart_kernel_log(
            "vfs: mknod: missing inode_ops.mknod at %s",
            basedir_path);
    }
    
    inode_t *new_node = basedir_inode->inode_ops->mknod(
        basedir_inode, new_child->name, mode, dev);

    if (new_node == NULL)
        return -1;

    // update dentry
    new_child->inode = new_node;
    return 0;
}

int32_t vfs_mkdir(const char *path, mode_t mode)
{
    mini_uart_kernel_log(
        "vfs: mkdir: path='%s', mode=0x%x",
        path, mode);

    // get the basedir path: TODO: handle path terminating with '/' here
    char basedir_path[256] = "";
    dentry_t *basedir = NULL;
    dentry_t *new_child = NULL;

    const int status = _check_new_path(path, basedir_path, &basedir, &new_child);
    if (status != 0)
        return status;

    KERNEL_ASSERT(basedir != NULL && basedir->inode != NULL);
    KERNEL_ASSERT(new_child != NULL && new_child->inode == NULL);

    // call fs backend.
    inode_t *basedir_inode = basedir->inode;
    if (basedir_inode->inode_ops == NULL ||
        basedir_inode->inode_ops->mkdir == NULL) {
        mini_uart_kernel_log(
            "vfs: mkdir: missing inode_ops.mkdir at %s",
            basedir_path);
    }
    
    inode_t *new_node = basedir_inode->inode_ops->mkdir(
        basedir_inode, new_child->name, mode);

    if (new_node == NULL)
        return -1;

    // update dentry
    new_child->inode = new_node;

    return 0;
}

//
//  VFS: File interface
//

file_t *vfs_file_open(const char *path, uint32_t flags, mode_t mode)
{
    (void)flags;
    (void)mode;

    mini_uart_kernel_log("vfs: open %s", path);
    dentry_t *dentry = _vfs_dentry_lookup(path);

    // of no dentry or a negative dentry
    if (dentry == NULL ||
        dentry->inode == NULL)
    {
        mini_uart_kernel_log(
            "vfs: open: no file at path '%s'", path);
        return NULL;
    }
    else if(
        dentry->inode->file_ops == NULL ||
        dentry->inode->file_ops->open == NULL)
    {
        mini_uart_kernel_log(
            "vfs: open: inode at '%s' does not implement open file operation",
            path);
        return NULL;
    }

    return dentry->inode->file_ops->open(dentry->inode);
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
