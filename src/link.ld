ENTRY (_start)

SECTIONS
{
    . = 0x40008000;
    _KERNEL_PA_BEGIN = .;
    _OFFSET = 0xffffffff00000000;

    .boot :  {
        KEEP(*start.o(.text* .data* .bss* COMMON* .rodata*))
        KEEP (*print_error.o(.text* .data* .bss* COMMON* .rodata*))
        KEEP(*uart_boot.o(.text* .data* .bss* COMMON* .rodata*))
        KEEP(*boot.o(.text* .data* .bss* COMMON* .rodata*))
    }

    _KERNEL_PA_END = .;

    . = ALIGN(2K);
    _dummy_load_address = .;

    .dummy 0x10000: AT(_dummy_load_address) {
        _dummy_start_va = .;
        _dummy_text_start_va = .;
        KEEP(*dummy.o(.text*))
        _dummy_text_end_va = .;
        KEEP(*dummy.o(.rodata* .data* .bss*))
        _dummy_end_va = .;
    }

    . = _dummy_load_address + (_dummy_end_va - 0x10000);
    _dummy_end = . + _OFFSET;
    _dummy_start = _dummy_load_address + _OFFSET;
    _dummy_text_start = (_dummy_load_address + (_dummy_text_start_va - _dummy_start_va)) + _OFFSET;
    _dummy_text_end = (_dummy_load_address + (_dummy_text_end_va - _dummy_start_va)) + _OFFSET;

    _dummy_text_start = _dummy_text_start_va;
    . += _OFFSET;
    . = ALIGN(4K);
    _KERNEL_VA_BEGIN = .;

    .text : AT(ADDR(.text) - _OFFSET){ *(.text*) *(.rodata*) }
    .data : AT(ADDR(.data) - _OFFSET){ *(.data*) }
    .bss : AT(ADDR(.bss) - _OFFSET){ *(.bss*) *(COMMON*) }

    . = ALIGN(2K);
    .exception : AT(ADDR(.exception) - _OFFSET) {
        _exception_vector_address = .;
        *(.exception*)
    }
    
    _KERNEL_VA_END = .;

    . = ALIGN(4K);
    _end_va = .;
    _end_pa = _end_va - _OFFSET;
}

