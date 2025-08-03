

int dummy;

// #include "zero.h"
// #include "lib/str.h"
// #include "vfs/vfs.h"

// static int32_t _dev_zero_read(void *_back, void *_ctx, void *data, size_t size)
// {
//     (void)_back;
//     (void)_ctx;
//     _memset(data, 0, size);
//     return size;
// }

// static int32_t _dev_zero_write(void *_back, void *_ctx, const void *data, size_t size)
// {
//     (void)_back;
//     (void)_ctx;
//     (void)data;
//     return size;  
// }

// file_handle_t vfs_dev_zero_create_handler(void)
// {
//     const file_handle_t handle = {
//         .backend = NULL,
//         .ops = {
//             .create_ctx = NULL,
//             .close_ctx = NULL,
//             .read = _dev_zero_read,
//             .write = _dev_zero_write,
//             .seek = NULL
//         }
//     };
//     return handle;
// }
