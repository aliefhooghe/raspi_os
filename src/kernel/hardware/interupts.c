#include "interupts.h"
#include "hardware/mmio.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"

uint32_t software_interupt_handler(
    uint32_t syscall_num,
    uint32_t program_status,
    uint32_t arg0, uint32_t arg1)
{
    (void)syscall_num;
    (void)program_status;
    (void)arg0;
    (void)arg1;

    return syscall_num * 100 + 10 * arg0 + arg1;
}



void irq_handler(void)
{
    unsigned int rb,rc;
    mini_uart_putc('X');


    // goal: empty the rx buffer
    // an interrupt has occurred, find out why
    while(1) //resolve all interrupts to uart
    {
        rb=mmio_read(REG__AUX_MU_IIR_REG);
        if((rb&1)==1) break; //no more interrupts
        if((rb&6)==4)
        {
            //receiver holds a valid byte
            rc=mmio_read(REG__AUX_MU_IO_REG); //read byte from rx fifo
            // rxbuffer[rxhead]=rc&0xFF;
            // rxhead=(rxhead+1)&RXBUFMASK;
        }
    }
}
