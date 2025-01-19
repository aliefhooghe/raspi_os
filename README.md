doc:
    https://developer.arm.com/documentation/ddi0301/latest/



sources:
- https://wiki.osdev.org/Raspberry_Pi_Bare_Boneshttps://wiki.osdev.org/Raspberry_Pi_Bare_Bones
- https://github.com/dwelch67/raspberrypi-zero
- https://leiradel.github.io/2019/02/10/The-Mini-UART.html


# On status register
- http://lioncash.github.io/ARMBook/the_apsr,_cpsr,_and_the_difference_between_them.html
- https://courses.washington.edu/cp105/02_Exceptions/Status_Registers.html
- https://www.snaums.de/informatik/controlling-features-of-the-arm1176jzf-s-raspberry-pi.html



.globl cpu_set_execution_mode
cpu_set_execution_mode:
    push {r1}

    mrs r1, cpsr    // read current cpsr
    bic r1, #0x1f   // clear mode bits
    orr r1, r1, r0  // set mode bits to given value
    msr cpsr, r1    // write result to cpsr

    pop {r1}
    bx lr


void cpu_set_execution_mode(uint8_t mode);