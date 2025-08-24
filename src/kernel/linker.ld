
ENTRY(_start)

MEMORY
{
    ram : ORIGIN = 0x8000, LENGTH = 0x80000 /* 512K - max is 7.9 Mb */
}

SECTIONS
{
    .text : { *(.text*) } > ram
    .rodata : { *(.rodata) } > ram
    .data : { *(.rodata) } > ram
    .bss : { *(.bss*) } > ram
    _end = .;

    ASSERT(
        _end - _start <= LENGTH(ram),
        "KERNEL DOES NOT FIT IN RAM REGION")
}
