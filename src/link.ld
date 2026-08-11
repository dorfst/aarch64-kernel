ENTRY (_start)

SECTIONS
{
    . = 0x40008000;
    _KERNEL_PA_BEGIN = .;
    _OFFSET = 0xffffffff00000000;

    .boot :  {
        KEEP(*start.o(.text* .data* .bss* COMMON* .rodata*))
        . = ALIGN(2K);
        _EXCEPTION_VECTOR_PA = .;
        KEEP(*exception_vector.o(.text* .data* .bss* COMMON* .rodata*))
        KEEP (*print_error.o(.text* .data* .bss* COMMON* .rodata*))
        KEEP(*uart_boot.o(.text* .data* .bss* COMMON* .rodata*))
        KEEP(*boot.o(.text* .data* .bss* COMMON* .rodata*))
    }

    _KERNEL_PA_END = .;


    . += _OFFSET;
    . = ALIGN(4K);
    _KERNEL_VA_BEGIN = .;

    .text : AT(ADDR(.text) - _OFFSET){ *(.text*) *(.rodata*) }
    .data : AT(ADDR(.data) - _OFFSET){ *(.data*) }
    .bss : AT(ADDR(.bss) - _OFFSET){ *(.bss*) *(COMMON*) }
    
    _KERNEL_VA_END = .;

    . = ALIGN(4K);
    _end_va = .;
    _end_pa = _end_va - _OFFSET;
}

