
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmio.h"
#include "hardware/mmu.h"
#include "hardware/watchdog.h"


#include "memory/allocator.h"
#include "memory/section_allocator.h"
#include "scheduler/scheduler.h"


#include "usermode/usermode.h"
#include "vfs/vfs.h"

extern const char *__satan_welcome_banner;
extern const char *__satan_fatal_error_banner;


#define KERNEL_HEAP_BEGIN           0x00100000u
#define KERNEL_HEAP_END             0x00800000u

#define KERNEL_DYN_SECTIONS_BEGIN   0x00800000u
#define KERNEL_DYN_SECTIONS_END     0x04800000u

static void kernel_init(void)
{
    section_allocator_init(KERNEL_DYN_SECTIONS_BEGIN);
    memory_allocator_init(KERNEL_HEAP_BEGIN, KERNEL_HEAP_END);
    scheduler_init();
    vfs_init();
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

    // disable aux (mini uart) interuptions (using polling for now)
    mmio_write(REG__IRQ_DISABLE_1, IRQ1_AUX_INT);


    mini_uart_puts("[kernel] Ready to start the mmu. Ready ?\r\n");
    mini_uart_getc();

    mini_uart_puts("[kernel] Really ?\r\n");
    mini_uart_getc();

    mmu_init();

    for (uint32_t i = 0u; i < 32u; i++)
    {
        mini_uart_printf("[kernel] I am alieve with the mmu %u\n\r", i);
    }

    // start user mode
    mini_uart_puts("[kernel] call user mode !\r\n");

    // bad: duplicated code
    void *first_user_stack = mem_alloc(0x1000u);
    scheduler_add_task((uintptr_t)user_function, first_user_stack, 0);
    scheduler_start();
}

void kernel_fatal_error(const char *reason)
{
    mini_uart_puts(__satan_fatal_error_banner);
    mini_uart_printf("[kernel] Fatal Satan failure:  %s\n\n[kernel] press a key...\r\n", reason);
    mini_uart_getc();
    watchdog_init(0x0);
}
