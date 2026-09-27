mov x3, #0
mov x4, #0
b loop

loop:
    bl check_for_yield
    cmp x2, x3
    add x3, x3, #1
    add x4, x4, #1
    b.gt loop
    b quit

check_for_yield:
    cmp x4, #4
    b.eq yield
    ret

yield:
    mov x4, #0
    mov x8, #20
    svc #0
    ret


quit:
    mov x8, #21
    svc #0

