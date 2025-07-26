
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"

#include "hardware/cpu.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmu.h"
#include "hardware/watchdog.h"

#include "memory/allocator.h"
#include "memory/section_allocator.h"
#include "memory/translation_table_allocator.h"

#include "scheduler/scheduler.h"

#include "usermode/usermode.h"
#include "vfs/vfs.h"

extern const char *__satan_welcome_banner;
extern const char *__satan_fatal_error_banner;

//  +-----------------+--------------------+----------------------+
//  | section         | size               | position             |
//  +-----------------+--------------------+----------------------+
//  | ◘ FIR stack     | 00001000 = 4.0KiB  | 00000000 -> 00001000 |
//  | ◘ IRQ stack     | 00001000 = 4.0KiB  | 00001000 -> 00002000 |
//  | ◘ SVC stack     | 00001000 = 4.0KiB  | 00002000 -> 00003000 |
//  | ◘ KERNEL        | 00018000 = 96.0KiB | 00008000 -> 00020000 |
//  | ◘ KERNEL HEAP   | 00700000 = 7.0MiB  | 00100000 -> 00800000 |
//  | ◘ DYN SECTIONS  | 04000000 = 64.0MiB | 00800000 -> 04800000 |
//  +-----------------+--------------------+----------------------+
//  | TOTAL           | 72.0MiB            |
//  +-----------------+--------------------+

//  KERNEL CODE + DATA = 0x00000000 - 0x00100000
//                     = the first section of 1Mb

#define KERNEL_HEAP_BEGIN           0x00100000u
#define KERNEL_HEAP_END             0x00800000u

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
    // Initialize the section allocator
    section_allocator_init(KERNEL_DYN_SECTIONS_BEGIN);

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

    // Initialize the memory allocator: to be removed
    memory_allocator_init(KERNEL_HEAP_BEGIN, KERNEL_HEAP_END);

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

    // initialize the mini UART
    mini_uart_init();

    // wait a first input
    mini_uart_puts(
        "[kernel] starting satan OS...\r\n"
            "[kernel] press a key...\r\n"
    );
    mini_uart_getc();

    // print a welcome message ;)
    mini_uart_puts(__satan_welcome_banner);

    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_puts("\r\n[kernel] System informations:\r\n");
    mini_uart_puts("[kernel] OS   : SATAN\r\n");
    mini_uart_puts("[kernel] CPU  : arm1176jzf-s\r\n");
    mini_uart_puts("[kernel] GPU  : RTX 6090 Satanic Edition\r\n");
    mini_uart_puts("[kernel] RAM  : 42 Go\r\n");
    mini_uart_puts("[kernel] Temp : 666°C\r\n");
    mini_uart_printf("[kernel] Mode : 0x%x\r\n", cpu_mode);

    // enable irq globaly
    cpu_irq_enable();

    // start user mode
    mini_uart_puts("[kernel] call user mode !\r\n");

    // bad: duplicated code
    scheduler_start((void*)user_function);
}

void kernel_fatal_error(const char *reason)
{
    mini_uart_puts(__satan_fatal_error_banner);
    mini_uart_printf("[kernel] Fatal Satan failure:  %s\n\n[kernel] press a key...\r\n", reason);
    mini_uart_getc();
    watchdog_init(0x0);
}

void kernel_unhandled_interupt_fatal_error(void)
{
    kernel_fatal_error("unhandled interupt"); 
}
