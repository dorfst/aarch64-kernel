.globl save_process

.equ PROC_CONTEXT_OFFSET, 16
.equ REG_SIZE, 8
.equ PROC_CONTEXT_L1_TABLE_OFFSET, 0
.equ PROC_CONTEXT_GP_OFFSET, 8
.equ PROC_CONTEXT_PC_OFFSET, 256
.equ PROC_CONTEXT_SP_OFFSET, 264
.equ PROC_CONTEXT_SPSR_OFFSET, 272

save_process:
    // save x0 on stack and load process context
    str x0, [sp, #-16]!
    adrp x0, kernel_state
    add x0, x0, :lo12:kernel_state
    add x0, x0, #(PROC_CONTEXT_OFFSET)

    str x1, [x0, #(PROC_CONTEXT_GP_OFFSET + 1 * REG_SIZE)]

    mrs x1, elr_el1
    str x1, [x0, #(PROC_CONTEXT_PC_OFFSET)]

    msr spsel, #0
    isb
    mov x1, sp
    str x1, [x0, #(PROC_CONTEXT_SP_OFFSET)]
    msr spsel, #1
    isb


    str x2, [x0, #(PROC_CONTEXT_GP_OFFSET + 2 * REG_SIZE)]
    str x3, [x0, #(PROC_CONTEXT_GP_OFFSET + 3 * REG_SIZE)]
    str x4, [x0, #(PROC_CONTEXT_GP_OFFSET + 4 * REG_SIZE)]
    str x5, [x0, #(PROC_CONTEXT_GP_OFFSET + 5 * REG_SIZE)]
    str x6, [x0, #(PROC_CONTEXT_GP_OFFSET + 6 * REG_SIZE)]
    str x7, [x0, #(PROC_CONTEXT_GP_OFFSET + 7 * REG_SIZE)]
    str x8, [x0, #(PROC_CONTEXT_GP_OFFSET + 8 * REG_SIZE)]
    str x9, [x0, #(PROC_CONTEXT_GP_OFFSET + 9 * REG_SIZE)]
    str x10, [x0, #(PROC_CONTEXT_GP_OFFSET + 10 * REG_SIZE)]
    str x11, [x0, #(PROC_CONTEXT_GP_OFFSET + 11 * REG_SIZE)]
    str x12, [x0, #(PROC_CONTEXT_GP_OFFSET + 12 * REG_SIZE)]
    str x13, [x0, #(PROC_CONTEXT_GP_OFFSET + 13 * REG_SIZE)]
    str x14, [x0, #(PROC_CONTEXT_GP_OFFSET + 14 * REG_SIZE)]
    str x15, [x0, #(PROC_CONTEXT_GP_OFFSET + 15 * REG_SIZE)]
    str x16, [x0, #(PROC_CONTEXT_GP_OFFSET + 16 * REG_SIZE)]
    str x17, [x0, #(PROC_CONTEXT_GP_OFFSET + 17 * REG_SIZE)]
    str x18, [x0, #(PROC_CONTEXT_GP_OFFSET + 18 * REG_SIZE)]
    str x19, [x0, #(PROC_CONTEXT_GP_OFFSET + 19 * REG_SIZE)]
    str x20, [x0, #(PROC_CONTEXT_GP_OFFSET + 20 * REG_SIZE)]
    str x21, [x0, #(PROC_CONTEXT_GP_OFFSET + 21 * REG_SIZE)]
    str x22, [x0, #(PROC_CONTEXT_GP_OFFSET + 22 * REG_SIZE)]
    str x23, [x0, #(PROC_CONTEXT_GP_OFFSET + 23 * REG_SIZE)]
    str x24, [x0, #(PROC_CONTEXT_GP_OFFSET + 24 * REG_SIZE)]
    str x25, [x0, #(PROC_CONTEXT_GP_OFFSET + 25 * REG_SIZE)]
    str x26, [x0, #(PROC_CONTEXT_GP_OFFSET + 26 * REG_SIZE)]
    str x27, [x0, #(PROC_CONTEXT_GP_OFFSET + 27 * REG_SIZE)]
    str x28, [x0, #(PROC_CONTEXT_GP_OFFSET + 28 * REG_SIZE)]
    str x29, [x0, #(PROC_CONTEXT_GP_OFFSET + 29 * REG_SIZE)]
    str x30, [x0, #(PROC_CONTEXT_GP_OFFSET + 30 * REG_SIZE)]
    // take x0 off the stack and save it into the context
    ldr x1, [sp], 16
    str x1, [x0, #(PROC_CONTEXT_GP_OFFSET + 0 * REG_SIZE)]
    b syscall_router