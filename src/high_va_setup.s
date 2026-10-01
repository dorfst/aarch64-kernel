.comm high_va_stack, 0x10000, 16

.globl high_va_setup
high_va_setup:
    msr spsel, #1
    adrp x0, high_va_stack
    add x0, x0, :lo12:high_va_stack
    add x0, x0, #0x10000
    mov sp, x0
    bl main
    b halt

.globl halt
halt:
    wfi
    b halt
