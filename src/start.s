.comm stack, 0x10000

.globl _start
_start:
    msr spsel, #1
    adrp x1, stack
    add x1, x1, :lo12:stack
    add x1, x1, #0x10000
    mov sp, x1
    bl boot
    b high_va_setup
    
