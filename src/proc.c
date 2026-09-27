#include "proc.h"
#include "mem.h"
#include <stdint.h>
#include <stdbool.h>

extern void load_process(struct proc_context* context);
extern void save_process(struct proc_context* context);

bool queue_is_full(struct proc_queue* queue) {
    // queue is of size 10
    return queue->back == 9;
}

bool queue_is_empty(struct proc_queue* queue) {
    return queue->back == -1;
}

void shift_queue(struct proc_queue* queue) {
    struct proc front = queue->queue[0];
    for (int64_t i = 0; i < queue->back; ++i) {
        queue->queue[i] = queue->queue[i + 1];
    }
    queue->queue[queue->back] = front;
}

void push_queue(struct proc_queue* queue, struct proc proc) {
    if (!queue_is_full(queue)) {
        ++queue->back;
        queue->queue[queue->back] = proc;
    }
}

void pop_queue(struct proc_queue* queue) {
    if (!queue_is_empty(queue)) {
        shift_queue(queue);
        --queue->back;
    }
}

struct proc proc_create(uint8_t pid, uint64_t x2_val, struct proc_queue* proc_queue, struct copy_info* copy_info, struct physical_page* page_arr, size_t page_arr_size) {
    const uint64_t va_offset = 0xffffffff00000000;
    uint64_t* l1 = create_tables(pid, page_arr, page_arr_size, (size_t)copy_info->size);
    populate_tables(l1, (uint64_t*)copy_info->destination, copy_info->size);

    struct proc_context process_context;
    process_context.l1_table = l1;
    for (int i = 0; i < 31; ++i) {
        if (i != 2) {
            process_context.general_purpose[i] = 0;
        }
        else {
            process_context.general_purpose[i] = x2_val;
        }
    }
    process_context.l1_table = (uint64_t*)((uint64_t)l1 - va_offset);
    process_context.pc = 0;
    process_context.sp = 0;
    process_context.spsr = 0;

    struct proc process = {pid, READY, 0, process_context, (uint64_t*)copy_info->destination};

    push_queue(proc_queue, process);

    return process;
}

