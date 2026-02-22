#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "kernel_types.h"

#include "hardware/system_timer.h"
#include "hardware/cpu.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmu.h"
#include "hardware/sd_host/sd_host.h"
#include "hardware/watchdog.h"

#include "memory/memory_allocator.h"
#include "memory/section_allocator.h"
#include "memory/translation_table_allocator.h"

#include "scheduler/scheduler.h"

#include "vfs/device_ops.h"
#include "vfs/driver_registry.h"
#include "vfs/vfs.h"

//
// Kernel banners
// 
extern const char *__satan_welcome_banner;
extern const char *__satan_fatal_error_banner;

//
extern uint8_t __bss_start__;
extern uint8_t __bss_end__;

//
// # Kernel Memory Layout:
// 
// 
// +-----------------+--------------------+----------------------+
// | section         | size               | position             |
// +-----------------+--------------------+----------------------+
// | ◘ FIR stack     | 00001000 = 4.00KiB | 00000000 -> 00001000 |
// | ◘ IRQ stack     | 00001000 = 4.00KiB | 00001000 -> 00002000 |
// | ◘ SVC stack     | 00004000 = 16.0KiB | 00002000 -> 00006000 |
// | ◘ Kernel Code   | 007f8000 = 7.97MiB | 00008000 -> 00800000 |
// | ◘ Dyn Sections  | 04000000 = 64.0MiB | 00800000 -> 04800000 |
// +-----------------+--------------------+----------------------+
// | TOTAL           | 72.00MiB           |
// +-----------------+--------------------+
//
//
//  1 SECTION          = 0x00100000
//  KERNEL CODE + DATA = 0x00000000 - 0x00800000
//                     = the 8 first sections of 1Mb

#define KERNEL_CODE_START           0x00008000u
#define KERNEL_CODE_END             0x00800000u

#define KERNEL_DYN_SECTIONS_BEGIN   0x00800000u
#define KERNEL_DYN_SECTIONS_END     0x04800000u

static uint32_t *kernel_translation_table;

// Device Identifiers
#define DEV_BLK_RAMDISK             MAKE_DEV(0, 0)
#define DEV_BLK_SDCARD              MAKE_DEV(1, 0)

#define DEV_CHR_TTY                 MAKE_DEV(0, 0)

// Private functions
static void _kernel_clear_bss(void)
{
    const size_t bss_size = &__bss_end__ - &__bss_start__;
    _memset(&__bss_start__, 0u, bss_size);
}

static void _kernel_init_translation_table(uint32_t *table)
{
    // map the kernel code+data / heap / dynamic sections
    translation_table_add_identity_mapping(table,
        0x00000000u, KERNEL_DYN_SECTIONS_END,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_NONE);

    // map IO registers for the kernel
    // TODO: set le TEX et cie car nous ne voulons pas de mise en cache
    // ainsi que les bits C et B
    // et de réordonement des accès mémoire sur les mmios
    translation_table_add_identity_mapping(table,
        IO_REG_START, IO_REG_END,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_NONE);
}

static void _kernel_init(void)
{
    _kernel_clear_bss();

    // initialize the mini UART
    mini_uart_init();
    mini_uart_kernel_puts("Satan OS kernel is starting...\r\n");
    mini_uart_kernel_log("bss: %x -> %x", &__bss_start__, &__bss_end__);

    // initialize the sdio controller
    sdhost_init();

    // initialize the system timer
    system_timer_init();

    // Initialize the section allocator
    section_allocator_init(KERNEL_DYN_SECTIONS_BEGIN);

    // Initialize the general purpose memory allocator
    memory_allocator_init();

    // Initialize the translation table allocator
    mini_uart_kernel_log("initialize translation table allocator");
    void *process_translation_tables_section = section_allocator_alloc();
    translation_table_allocator_init(process_translation_tables_section);

    // Initialize the kernel translation table
    mini_uart_kernel_log("initialize kernel translation table");
    kernel_translation_table = translation_table_allocator_alloc();
    _kernel_init_translation_table(kernel_translation_table);

    // Enable and initialize the MMU with the kernel translation table
    mini_uart_kernel_log("enable MMU");
    kernel_restore_translation_table();
    mmu_set_dacr(0x55555555);
    mmu_enable();

    // Initialize the driver registry
    mini_uart_kernel_log("initialize drivers");
    driver_registry_init();

    // Initialize the scheduler
    mini_uart_kernel_log("initialize the scheduler");
    scheduler_init();

    // Initialize the Virtual File System
    mini_uart_kernel_log("initialize the vfs");
    vfs_init();
}

static void _kernel_mount_root_fs(void)
{
    //// Mount sdcard fat32 fs on /
    //
    // Note: sdcard is expected to be formated as a bare fat32 (no partition)
    //
    mini_uart_kernel_log("mount sdcard rootfs at /");
    block_device_t *sdcard_device = get_block_device(DEV_BLK_SDCARD);
    const int32_t root_mount_status = vfs_mount_dev(sdcard_device, "/", "fat32");
    KERNEL_ASSERT(root_mount_status == 0);
}

static void _kernel_mount_dev_tmpfs(void)
{
    mini_uart_kernel_log("mount dev tmpfs at /dev");

    // mount a tmpfs on /dev
    const int32_t dev_mount_status = vfs_mount(NULL, "/dev", "ramfs");
    KERNEL_ASSERT(0 == dev_mount_status);

    // create device files
    KERNEL_ASSERT(0 == vfs_mknod("/dev/tty", S_IFCHR, DEV_CHR_TTY));
    KERNEL_ASSERT(0 == vfs_mknod("/dev/ramdisk", S_IFBLK, DEV_BLK_RAMDISK));
    KERNEL_ASSERT(0 == vfs_mknod("/dev/sdcard", S_IFBLK, DEV_BLK_SDCARD));
}

//
// Public Kernel Functions
//

void kernel_restore_translation_table(void)
{
    mmu_set_translation_table(kernel_translation_table);
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    // initialize the kernel
    _kernel_init();

    // Prepare filesystem
    _kernel_mount_root_fs();
    _kernel_mount_dev_tmpfs();

    // wait a first input
    mini_uart_kernel_puts("Satan OS is initialized.\r\n");
    mini_uart_kernel_puts("press a key to continue ...\r\n");
    mini_uart_getc();

    // print a welcome message ;)
    mini_uart_kernel_puts(__satan_welcome_banner);

    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_kernel_log("cpu mode: 0x%x", cpu_mode);

    // start user mode
    mini_uart_kernel_log("call user mode init");
    scheduler_start("/bin/init");
}

void kernel_fatal_error(const char *reason)
{
    mini_uart_kernel_puts(__satan_fatal_error_banner);
    mini_uart_kernel_puts("Fatal Satan failure:  ");
    mini_uart_kernel_puts(reason);
    mini_uart_kernel_puts("\r\npress a key...");
    mini_uart_getc();

    // trigger a reboot
    watchdog_reboot();
}

void kernel_unhandled_interupt_fatal_error(void)
{
    kernel_fatal_error("unhandled interupt"); 
}
