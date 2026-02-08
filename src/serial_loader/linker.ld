
ENTRY(_start)

MEMORY
{
    /* Loader:
     * - boor at 0x8000
     * - copy itself to 0x7000-0x8000.
     * - max size is 0x1000
     */
    ram     : ORIGIN = 0x8000, LENGTH = 0x1000
}

SECTIONS
{

    . = ORIGIN(ram);
    __image_start = .;

    .text : {
        KEEP(*(.text.boot))
        *(.text*)
    } > ram

    .rodata : { *(.rodata*) } > ram
    .data   : { *(.data*)   } > ram
    .bss    : { *(.bss*)    } > ram

    __image_end = .;

    ASSERT(
        __image_end - __image_start  <= LENGTH(ram),
        "SERIAL LOADER DOES NOT FIT IN RAM REGION")
}
