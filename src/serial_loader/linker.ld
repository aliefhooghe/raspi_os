
ENTRY(_start)

MEMORY
{
    /* Loader:
     * - is loaded to 0x7000-0x8000.
     * - max size is 0x1000
     */
    ram     : ORIGIN = 0x8000, LENGTH = 0x1000
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
        "SERIAL LOADER DOES NOT FIT IN RAM REGION")
}
