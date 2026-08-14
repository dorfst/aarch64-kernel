ldr x3, #0
b loop

loop:
    cmp x2, x3
    add x3, x3, #1
    b.gt loop

