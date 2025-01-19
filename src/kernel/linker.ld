
ENTRY(_start)

MEMORY
{
    ram : ORIGIN = 0x8000, LENGTH = 0x10000
}

SECTIONS
{
    __start = .;

    __text_start = .;
    .text : { *(.text*) } > ram
    __text_end = .;

    __rodata_start = .;
    .rodata : { *(.rodata) } > ram
    __rodata_end = .;

    __data_start = .;
    .data : { *(.rodata) } > ram
    __data_end = .;

    __bss_start = .;
    .bss : { *(.bss*) } > ram
    __bss_end = .;
    __bss_size = __bss_end - __bss_start;

    __end = .;
}
