.section .exception, "ax", %progbits


exception_vector:
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    msr spsel, #1
    b save_process
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .
    .balign 0x80
    b .



.balign 2048
.globl syscall_router
syscall_router:
    cmp x8, #20
    b.eq yield_handler
    cmp x8, #21
    b.eq quit_handler
    b.ne .

