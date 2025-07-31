#include <stddef.h>
#include <stdint.h>

#include <stdnoreturn.h>

#include "kernel.h"

#include "hardware/cpu.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmu.h"
#include "hardware/watchdog.h"

#include "memory/memory_allocator.h"
#include "memory/section_allocator.h"
#include "memory/translation_table_allocator.h"

#include "scheduler/scheduler.h"

#include "usermode/usermode.h"

#include "vfs/vfs.h"

//
// Kernel banners
// 
extern const char *__satan_welcome_banner;
extern const char *__satan_fatal_error_banner;

//
// # Kernel Memory Layout:
// 
// 
// +-----------------+--------------------+----------------------+
// | section         | size               | position             |
// +-----------------+--------------------+----------------------+
// | ◘ FIR stack     | 00001000 = 4.00KiB | 00000000 -> 00001000 |
// | ◘ IRQ stack     | 00001000 = 4.00KiB | 00001000 -> 00002000 |
// | ◘ SVC stack     | 00001000 = 4.00KiB | 00002000 -> 00003000 |
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

static void kernel_init_translation_table(uint32_t *table)
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

static void kernel_init(void)
{
    // initialize the mini UART
    mini_uart_init();

    // Initialize the section allocator
    section_allocator_init(KERNEL_DYN_SECTIONS_BEGIN);

    // Initialize the general purpose memory allocator
    memory_allocator_init();

    // Initialize the translation table allocator
    void *process_translation_tables_section = section_allocator_alloc();
    translation_table_allocator_init(process_translation_tables_section);

    // Initialize the kernel translation table
    kernel_translation_table = translation_table_allocator_alloc();
    kernel_init_translation_table(kernel_translation_table);

    // Enable and initialize the MMU with the kernel translation table
    kernel_restore_translation_table();
    mmu_set_dacr(0x55555555);
    mmu_enable();

    // Initialize the scheduler
    scheduler_init();

    // Initialize the Virtual File System
    vfs_init();
}

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
    kernel_init();

    // wait a first input
    mini_uart_kernel_puts("starting satan OS...\r\n");
    mini_uart_kernel_puts("press a key...\r\n");
    mini_uart_getc();

    // print a welcome message ;)
    mini_uart_kernel_puts(__satan_welcome_banner);

    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_kernel_log("cpu mode: 0x%x", cpu_mode);

    // enable irq globaly
    cpu_irq_enable();

    // start user mode
    mini_uart_kernel_log("call user mode !");

    // duplicated code
    scheduler_start((void*)user_function);
}

noreturn void kernel_fatal_error(const char *reason)
{
    mini_uart_kernel_puts(__satan_fatal_error_banner);
    mini_uart_kernel_puts("Fatal Satan failure:  ");
    mini_uart_kernel_puts(reason);
    mini_uart_kernel_puts("\r\npress a key...");
    mini_uart_getc();
    watchdog_init(0x0);

    // hang. The watchdog will reset
    for (;;);
}

noreturn void kernel_unhandled_interupt_fatal_error(void)
{
    kernel_fatal_error("unhandled interupt"); 
}
