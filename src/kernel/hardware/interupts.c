

#include "hardware/cpu.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmio.h"

void irq_handler(void)
{
    unsigned int rb,rc;

    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_kernel_log("cpu mode: 0x%x\r\n", cpu_mode);

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
