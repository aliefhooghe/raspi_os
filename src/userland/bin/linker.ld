
ENTRY(_start)

MEMORY
{
    /* see scheduler.c */
    ram : ORIGIN = 0x00840000, LENGTH = 0x20000 /* 128 Kb */
}

SECTIONS
{
    .text : { *(.text*) } > ram
    .rodata : { *(.rodata*) } > ram
    .data : { *(.data*) } > ram
    .bss : { *(.bss*) } > ram
    _end = .;
    
    ASSERT(
        _end - _start <= LENGTH(ram),
        "USERLAND PROGRAM DOES NOT FIT IN RAM REGION")
}
