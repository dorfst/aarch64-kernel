.comm stack, 0x10000

_start:
    .globl _start
    msr spsel, #1
    ldr x1, =stack+0x10000
    mov sp, x1
    bl main
    
