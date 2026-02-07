
#include <stdint.h>

#include "hardware/mmu.h"
#include "lib/str.h"
#include "memory/section_allocator.h"
#include "ramdisk.h"
#include "vfs/device_ops.h"

#define SECTION_RAMDISK_BLOCK_COUNT  2048u
#define SECTION_RAMDISK_BLOCK_SIZE   512u

_Static_assert(
    MMU_SECTION_SIZE / SECTION_RAMDISK_BLOCK_SIZE == SECTION_RAMDISK_BLOCK_COUNT,
    "bad block count for section ramdisk" 
);

static int _section_ramdisk_read_block(void *private, uint32_t index, void *block)
{
    const uint8_t *section_base = (uint8_t*)private;
    const uint8_t *ram_block = section_base + index * SECTION_RAMDISK_BLOCK_SIZE;

    if (index >= SECTION_RAMDISK_BLOCK_COUNT) {
        return -1;
    }

    _memcpy(block, ram_block, SECTION_RAMDISK_BLOCK_SIZE);
    return 1;
}

static int _section_ramdisk_write_block(void *private, uint32_t index, const void *block)
{
    uint8_t *section_base = (uint8_t*)private;
    uint8_t *ram_block = section_base + index * SECTION_RAMDISK_BLOCK_SIZE;

    if (index >= SECTION_RAMDISK_BLOCK_COUNT) {
        return -1;
    }

    _memcpy(ram_block, block, SECTION_RAMDISK_BLOCK_SIZE);
    return 1;
}

static const block_device_ops_t _section_ramdisk_ops = {
    .read_block = _section_ramdisk_read_block,
    .write_block = _section_ramdisk_write_block
};

int create_ramdisk(block_device_t *device, void *mem, size_t size)
{
    (void)size; // TODO: save size and check reads
    device->ops = &_section_ramdisk_ops;
    device->block_size = SECTION_RAMDISK_BLOCK_SIZE ;
    device->private = mem;

    // TODO: horrible

    return 0;
}

int create_section_ramdisk(block_device_t *device)
{
    // allocate a section
    void *section = section_allocator_alloc();
    if (section == NULL) {
        return -1;
    }

    // create device
    device->ops = &_section_ramdisk_ops;
    device->block_size = SECTION_RAMDISK_BLOCK_SIZE ;
    device->private = section;

    return 0;
}
