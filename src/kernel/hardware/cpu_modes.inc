
.set CPU_MODE_USER, 0x10
.set CPU_MODE_FIQ,  0x11
.set CPU_MODE_IRQ,  0x12
.set CPU_MODE_SVC,  0x13
.set CPU_MODE_ABORT, 0x17
.set CPU_MODE_SYS,  0x1F

// Set the current cpu mode without affecting fiq/irq flags
.macro SET_CPU_MODE mode, reg=r12
    mrs \reg, cpsr
    bic \reg, \reg, #0x1F    // clear current cpu mode
    orr \reg, \reg, #\mode   // set cpu mode
    msr cpsr_c, r12
.endm

