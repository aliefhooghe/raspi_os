

#include "vfs/vfs.h"
#include "kernel.h"
#include "memory/bitfield.h"
#include "vfs/vfs_handler.h"
#include "vfs/dev/tty.h"

#include "memory/section_allocator.h"
#include "lib/str.h"


/**
 *  vfs
 */
#define NODE_NAME_MAX_SIZE (64u)
#define MAX_CHILD_COUNT    (10u)

typedef struct vfs_node {
    char name[NODE_NAME_MAX_SIZE];
    struct vfs_node *parent;
    file_handle_t handler;
    enum {
        VFS_NODE_FILE,
        VFS_NODE_DIRECTORY
    } type;

    union {
        void *file_backend;
        struct {
            struct vfs_node *childs[MAX_CHILD_COUNT];
            uint32_t child_count;
        } directory;  
    };
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
    void *memory_section;    // 1Mb
    uint8_t node_alloc_bitfield[NODE_ALLOCATOR_BITFIELD_COUNT];

    vfs_node_t *root_node;
} vfs_t;


/**
 *  global vfs
 */
static vfs_t _vfs;

//
// vfs internals
//
static file_descriptor_t _create_descriptor(file_handle_t *handle)
{
    const file_descriptor_t fd = {
        .handle = handle,
        .fd_ctx = NULL // TODO
    };
    return fd;
}

// TODO: reusable allocator
static vfs_node_t *_vfs_node_allocator_alloc(void)
{
    const int32_t node_index = bitfield_acquire_first(
        _vfs.node_alloc_bitfield,
        NODE_ALLOCATOR_BITFIELD_COUNT);
    if (node_index < 0)
        return NULL;
    else
        return (vfs_node_t*)((uint8_t*)_vfs.memory_section + node_index * sizeof(vfs_node_t));
}

static void _vfs_node_allocator_free(vfs_node_t *node)
{
    const uint32_t node_index = node - (vfs_node_t*)_vfs.memory_section;
    bitfield_clear(_vfs.node_alloc_bitfield, node_index);
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
    vfs_node_t *root_node = _vfs_node_allocator_alloc();
    if (root_node == NULL)
        kernel_fatal_error("vfs root node alloc failed");   
    _vfs.root_node = root_node;

    root_node->name[0] = '\0';
    root_node->parent = NULL;
    // root_node->handler =      
    root_node->type = VFS_NODE_DIRECTORY;

    // create an only child: /tty
    vfs_node_t *tty_node = _vfs_node_allocator_alloc();
    if (tty_node == NULL)
        kernel_fatal_error("vfs tty node alloc failed"); 
    root_node->directory.child_count = 1u;
    root_node->directory.childs[0] = tty_node;

    _strcpy(tty_node->name, "tty");
    tty_node->parent = root_node;
    tty_node->handler = tty_init_virtual_file();
    tty_node->type = VFS_NODE_FILE;
    tty_node->file_backend = NULL;  // no backend state for tty
}

file_descriptor_t vfs_get_tty_file_descriptor(void)
{
    return _create_descriptor(&_vfs.root_node->directory.childs[0]->handler);
}

//
//  File Descriptor interface
//
int32_t file_descriptor_read(file_descriptor_t *fd, void *data, size_t size)
{
    return fd->handle->ops.read(fd->handle->backend, fd->fd_ctx, data, size);
}

int32_t file_descriptor_write(file_descriptor_t *fd, const void *data, size_t size)
{
    return fd->handle->ops.write(fd->handle->backend, fd->fd_ctx, data, size);
}
