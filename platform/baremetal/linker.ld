ENTRY(_start)

MEMORY
{
    ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM (rw)  : ORIGIN = 0x00400000, LENGTH = 124M
}

SECTIONS
{
    . = ORIGIN(ROM);

    /* Code */
    .text : ALIGN(4)
    {
        *(.text.start)
        *(.text*)
        *(.rodata*)
    } > ROM

    /* Data */
    . = ORIGIN(RAM);

    .data : ALIGN(4)
    {
        *(.data*)
    } > RAM

    .bss : ALIGN(4)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    __stack_top = 0x08000000;
}
