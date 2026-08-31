.globl load_process

.equ REG_SIZE, 8
.equ PROC_CONTEXT_L1_TABLE_OFFSET, 0
.equ PROC_CONTEXT_GP_OFFSET, 8
.equ PROC_CONTEXT_PC_OFFSET, 256
.equ PROC_CONTEXT_SP_OFFSET, 264
.equ PROC_CONTEXT_SPSR_OFFSET, 272

load_process:
    ldr x1, [x0, #(PROC_CONTEXT_L1_TABLE_OFFSET)]
    msr ttbr0_el1, x1
    isb
    tlbi vmalle1
    dsb ish
    isb

    ldr x1, [x0, #(PROC_CONTEXT_PC_OFFSET)]
    msr elr_el1, x1

    ldr x1, [x0, #(PROC_CONTEXT_SPSR_OFFSET)]
    msr spsr_el1, x1

    ldr x1, [x0, #(PROC_CONTEXT_SP_OFFSET)]
    msr spsel, #0
    isb
    mov sp, x1

    ldr x1, [x0, #(PROC_CONTEXT_GP_OFFSET + 1 * REG_SIZE)]
    ldr x2, [x0, #(PROC_CONTEXT_GP_OFFSET + 2 * REG_SIZE)]
    ldr x3, [x0, #(PROC_CONTEXT_GP_OFFSET + 3 * REG_SIZE)]
    ldr x4, [x0, #(PROC_CONTEXT_GP_OFFSET + 4 * REG_SIZE)]
    ldr x5, [x0, #(PROC_CONTEXT_GP_OFFSET + 5 * REG_SIZE)]
    ldr x6, [x0, #(PROC_CONTEXT_GP_OFFSET + 6 * REG_SIZE)]
    ldr x7, [x0, #(PROC_CONTEXT_GP_OFFSET + 7 * REG_SIZE)]
    ldr x8, [x0, #(PROC_CONTEXT_GP_OFFSET + 8 * REG_SIZE)]
    ldr x9, [x0, #(PROC_CONTEXT_GP_OFFSET + 9 * REG_SIZE)]
    ldr x10, [x0, #(PROC_CONTEXT_GP_OFFSET + 10 * REG_SIZE)]
    ldr x11, [x0, #(PROC_CONTEXT_GP_OFFSET + 11 * REG_SIZE)]
    ldr x12, [x0, #(PROC_CONTEXT_GP_OFFSET + 12 * REG_SIZE)]
    ldr x13, [x0, #(PROC_CONTEXT_GP_OFFSET + 13 * REG_SIZE)]
    ldr x14, [x0, #(PROC_CONTEXT_GP_OFFSET + 14 * REG_SIZE)]
    ldr x15, [x0, #(PROC_CONTEXT_GP_OFFSET + 15 * REG_SIZE)]
    ldr x16, [x0, #(PROC_CONTEXT_GP_OFFSET + 16 * REG_SIZE)]
    ldr x17, [x0, #(PROC_CONTEXT_GP_OFFSET + 17 * REG_SIZE)]
    ldr x18, [x0, #(PROC_CONTEXT_GP_OFFSET + 18 * REG_SIZE)]
    ldr x19, [x0, #(PROC_CONTEXT_GP_OFFSET + 19 * REG_SIZE)]
    ldr x20, [x0, #(PROC_CONTEXT_GP_OFFSET + 20 * REG_SIZE)]
    ldr x21, [x0, #(PROC_CONTEXT_GP_OFFSET + 21 * REG_SIZE)]
    ldr x22, [x0, #(PROC_CONTEXT_GP_OFFSET + 22 * REG_SIZE)]
    ldr x23, [x0, #(PROC_CONTEXT_GP_OFFSET + 23 * REG_SIZE)]
    ldr x24, [x0, #(PROC_CONTEXT_GP_OFFSET + 24 * REG_SIZE)]
    ldr x25, [x0, #(PROC_CONTEXT_GP_OFFSET + 25 * REG_SIZE)]
    ldr x26, [x0, #(PROC_CONTEXT_GP_OFFSET + 26 * REG_SIZE)]
    ldr x27, [x0, #(PROC_CONTEXT_GP_OFFSET + 27 * REG_SIZE)]
    ldr x28, [x0, #(PROC_CONTEXT_GP_OFFSET + 28 * REG_SIZE)]
    ldr x29, [x0, #(PROC_CONTEXT_GP_OFFSET + 29 * REG_SIZE)]
    ldr x30, [x0, #(PROC_CONTEXT_GP_OFFSET + 30 * REG_SIZE)]
    ldr x0, [x0, #(PROC_CONTEXT_GP_OFFSET + 0 * REG_SIZE)]
    eret