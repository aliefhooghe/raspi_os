

#include "hardware/mini_uart.h"

#include "kernel.h"
#include "kernel_types.h"
#include "lib/str.h"

#include "memory/bitfield.h"
#include "memory/memory_allocator.h"
#include "memory/section_allocator.h"

#include "vfs/dev/tty.h"
#include "vfs/vfs.h"
#include "vfs/dev/zero.h"
#include "vfs/vfs_handler.h"

/**
 *  vfs
 */
#define NODE_NAME_MAX_SIZE (52u)
#define MAX_CHILD_COUNT    (10u)

typedef enum {
    VFS_NODE_FILE,
    VFS_NODE_DIRECTORY
} vfs_node_type_t;

typedef struct {
    struct vfs_node *childs[MAX_CHILD_COUNT];
    uint32_t child_count;
} vfs_directory_t;  

typedef struct vfs_node {
    char name[NODE_NAME_MAX_SIZE];
    struct vfs_node *parent;
    file_handle_t handler;
    vfs_node_type_t type;
    vfs_directory_t directory; // bad for FILES?????????????????????????, 
} vfs_node_t;

_Static_assert(
    sizeof(vfs_node_t) == 128u,
    "vfs node size is assumed to be 128");

// 8 * 8 = 64 bits. Can manage up to 64 vfs nodes
#define NODE_ALLOCATOR_BITFIELD_COUNT 8u 
#define NODE_ARRAY_SIZE (sizeof(vfs_node_t) * NODE_ALLOCATOR_BITFIELD_COUNT * 8u)

_Static_assert(
    NODE_ARRAY_SIZE == 8192,
    "node array size is assumed to be 8192 bytes"
);

typedef struct {
    // memory management
    void *memory_section;                                       // 1Mb
    uint8_t node_alloc_bitfield[NODE_ALLOCATOR_BITFIELD_COUNT]; // 8 kb = 1 section / 128

    // tree structure
    vfs_node_t *root_node;
} vfs_t;


//
// global vfs
//
static vfs_t _vfs;

//
// vfs internals
//
static file_descriptor_t _create_null_descriptor(void)
{
    const file_descriptor_t null_fd = {
        .handle = NULL
    };
    return null_fd;
}

static file_descriptor_t _create_descriptor(const vfs_node_t *node)
{
    const file_descriptor_t fd = {
        .handle = &node->handler,
        .fd_ctx = node->handler.ops.create_ctx == NULL ? NULL :
            node->handler.ops.create_ctx(node->handler.backend)
    };
    return fd;
}

static void _close_descriptor(file_descriptor_t *descriptor)
{
    if (descriptor->handle->ops.close_ctx != NULL)
         descriptor->handle->ops.close_ctx(
            descriptor->handle->backend,
            descriptor->fd_ctx);
    descriptor->handle = NULL;
    descriptor->fd_ctx = NULL;
}
//
// Node management 
// 

// node allocation
static vfs_node_t *_vfs_node_allocator_alloc(void)
{
    // TODO: reusable allocator
    const int32_t node_index = bitfield_acquire_first(
        _vfs.node_alloc_bitfield,
        NODE_ALLOCATOR_BITFIELD_COUNT);
    if (node_index < 0)
        return NULL;
    else
        return (vfs_node_t*)((uint8_t*)_vfs.memory_section + node_index * sizeof(vfs_node_t));
}

// static void _vfs_node_allocator_free(vfs_node_t *node)
// {
//     const uint32_t node_index = node - (vfs_node_t*)_vfs.memory_section;
//     bitfield_clear(_vfs.node_alloc_bitfield, node_index);
// }

// node initialization
static vfs_node_t *_vfs_node_create(
    const char *name,
    vfs_node_t *parent,
    vfs_node_type_t type,
    const file_handle_t *handler)
{
    vfs_node_t *node = _vfs_node_allocator_alloc();
    if (node == NULL)
        kernel_fatal_error("vfs node alloc failed"); 

    _strcpy(node->name, name);
    node->parent = parent;
    node->type = type;

    if (handler == NULL) {
        _memset(&node->handler, 0u, sizeof(file_handle_t));
    }
    else {
        node->handler = *handler;
    }

    return node;
}

// node lookup
static const vfs_node_t *_vfs_child_by_name(
    const vfs_node_t *parent,
    const char *child_name,
    uint32_t child_name_len)
{
    for (size_t index = 0u; index < parent->directory.child_count; index++)
    {
        const vfs_node_t *child = parent->directory.childs[index];
        if (0 == _strncmp(child_name, child->name, child_name_len))
            return child;
    }
    return NULL;
}

static const vfs_node_t *_vfs_node_lookup_rec(
    const char *path,
    const vfs_node_t *root)
{
    mini_uart_kernel_log(
        "vfs: node lookup: look for relative path '%s' in node %s",
        path, root->name);
    const char *next_separator = _strchr(path, '/');
    if (*next_separator == '\0') {
        // then path itself is the last segment
        const uint32_t child_name_len = _strlen(path);
        return _vfs_child_by_name(root, path, child_name_len);
    }
    else {
        const uint32_t child_name_len = next_separator - path;
        const vfs_node_t *child = _vfs_child_by_name(root, path, child_name_len);       
        if (child == NULL || child->type == VFS_NODE_FILE)
            return NULL;
        return _vfs_node_lookup_rec(path + child_name_len + 1, child);
    }
}

static const vfs_node_t *_vfs_node_lookup(const char *path)
{
    if (path[0] != '/')
        return NULL;
    const size_t path_len = _strlen(path);
    mini_uart_kernel_log("node lookup: path='%s', len=%u", path, path_len);
    return _vfs_node_lookup_rec(path + 1, _vfs.root_node);
}

//
// Directory handlers
// 

typedef struct {
    uint32_t cur;
} vfs_directory_fd_context_t;

static void *_vfs_directory_handler_create_ctx(void *backend)
{
    (void)backend;
    const size_t ctx_size = sizeof(vfs_directory_fd_context_t);
    vfs_directory_fd_context_t *ctx = (vfs_directory_fd_context_t*)memory_alloc(ctx_size);
    _memset(ctx, 0u, ctx_size);
    return ctx;
}

static void _vfs_directory_handler_close_ctx(void *backend, void *ctx)
{
    (void)backend;
    memory_free(ctx);
}

static int32_t _vfs_directory_handler_read(void *backend, void *ctx, void *data, size_t size)
{
    vfs_directory_fd_context_t *fd_ctx = (vfs_directory_fd_context_t*)ctx;
    vfs_directory_t *directory = (void*)backend;

    struct dirent *entities = (dirent *)data;
    const size_t total_buffer_size = size;

    for (
        ;
        fd_ctx->cur < directory->child_count && size >= sizeof(dirent);
        fd_ctx->cur++, size -= sizeof(dirent))
    {
        const vfs_node_t *vfs_entity = directory->childs[fd_ctx->cur];
        _strcpy(entities[fd_ctx->cur].d_name, vfs_entity->name);
        entities[fd_ctx->cur].d_type = (vfs_entity->type == VFS_NODE_DIRECTORY);
    }

    // note: if cur after end, return 0 => unix compliant
    return total_buffer_size - size;
}

static int32_t _vfs_directory_handler_write(void *backend, void *ctx, const void *data, size_t size)
{
    (void)backend;
    (void)ctx;
    (void)data;
    (void)size;
    kernel_fatal_error("write on directory is not implemented");
    return -1;
}

static int32_t _vfs_directory_handler_seek(void *backend, void *ctx, int32_t offset, int32_t whence)
{
    _Static_assert(
        sizeof(dirent) == 32,
        "sizeof(dirent) is expected to be 32");

    vfs_directory_fd_context_t *fd_ctx = (vfs_directory_fd_context_t*)ctx;
    vfs_directory_t *directory = (void*)backend;

    int32_t index_ref;
    switch (whence) {
        case SEEK_SET: index_ref = 0u; break;
        case SEEK_CUR: index_ref = fd_ctx->cur; break;
        case SEEK_END: index_ref = directory->child_count; break;
        default: return -1;
    }

    const int32_t index_offset = offset >> 0x5u;
    const int32_t new_cur = index_ref + index_offset;

    // position before begin is not allowed
    if (new_cur < 0)
        return -1;

    // position after end is ok.
    fd_ctx->cur = new_cur;
    return new_cur * sizeof(dirent);
}

static file_handle_t _vfs_directory_create_handler(vfs_directory_t *directory)
{
    const file_handle_t handler = {
        .ops = {
            .create_ctx = _vfs_directory_handler_create_ctx,
            .close_ctx = _vfs_directory_handler_close_ctx,
            .read = _vfs_directory_handler_read,
            .write = _vfs_directory_handler_write,
            .seek = _vfs_directory_handler_seek
        },
        .backend = directory
    };
    return handler;
}

static vfs_node_t *_vfs_create_directory_node(vfs_node_t *parent, const char *name)
{
    vfs_node_t *dir = _vfs_node_create(name, parent, VFS_NODE_DIRECTORY, NULL);
    dir->handler = _vfs_directory_create_handler(&dir->directory);
    return dir;
}

//
// vfs interface
//
void vfs_init(void)
{
    _memset(&_vfs, 0, sizeof(vfs_t));

    // allocate memory section for vfs
    _vfs.memory_section = section_allocator_alloc();
    if (_vfs.memory_section == NULL)
        kernel_fatal_error("vfs memory section allocation failed");

    // create root descriptor
    vfs_node_t *root = _vfs_create_directory_node(NULL, "root");

    //
    // create the /dev node
    //
    vfs_node_t *dev = _vfs_create_directory_node(root, "dev");
    

    // zero device
    file_handle_t zero_handler = vfs_dev_zero_create_handler();
    vfs_node_t *zero = _vfs_node_create("zero", dev, VFS_NODE_FILE, &zero_handler);
    

    // tty device
    file_handle_t tty_handler = vfs_dev_tty_create_handler();
    vfs_node_t *tty = _vfs_node_create("tty", dev, VFS_NODE_FILE, &tty_handler);

    // hierarchy
    root->directory.child_count = 1u;
    root->directory.childs[0] = dev;

    dev->directory.child_count = 2u;
    dev->directory.childs[0] = tty;
    dev->directory.childs[1] = zero;

    // set the vfs root
    _vfs.root_node = root;
}

//
//  File Descriptor interface
//
int32_t vfs_file_descriptor_is_null(const file_descriptor_t *desc)
{
    return desc == NULL || desc->handle == NULL;
}

file_descriptor_t vfs_file_descriptor_open(const char *path, uint32_t flags, uint32_t mode)
{
    (void)flags;
    (void)mode;
    mini_uart_kernel_log("vfs: open %s", path);

    const vfs_node_t *node = _vfs_node_lookup(path);
    if (node == NULL)
    {
        mini_uart_kernel_log("vfs: lookup failed for path: %s", path);
        return _create_null_descriptor();
    }
    else
    {
        mini_uart_kernel_log("vfs: lookup succeed for path: %s", path);
    }

    return _create_descriptor(node); 
}

int32_t vfs_file_descriptor_close(file_descriptor_t *fd)
{
    _close_descriptor(fd);
    return 0;
}

int32_t vfs_file_descriptor_read(file_descriptor_t *fd, void *data, size_t size)
{
    return fd->handle->ops.read(fd->handle->backend, fd->fd_ctx, data, size);
}

int32_t vfs_file_descriptor_write(file_descriptor_t *fd, const void *data, size_t size)
{
    return fd->handle->ops.write(fd->handle->backend, fd->fd_ctx, data, size);
}

int32_t vfs_file_descriptor_lseek(file_descriptor_t *fd, int32_t offset, int32_t whence)
{
    // seek is allowed to not be implemented by the handler
    if (fd->handle->ops.seek == NULL)
        return -1;
    return fd->handle->ops.seek(fd->handle->backend, fd->fd_ctx, offset, whence);
}
