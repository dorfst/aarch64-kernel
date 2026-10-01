#ifndef PROC
#define PROC
#include "mem.h"
#include <stdint.h>
#include <stdbool.h>



enum PROC_STATE {
    READY, RUNNING, DEAD
};



struct proc_context {
    uint64_t* l1_table;
    uint64_t general_purpose[31];
    uint64_t pc;
    uint64_t sp;
    uint64_t spsr;
};

struct proc {
    uint64_t pid;
    enum PROC_STATE state;
    uint64_t exec_time;
    struct proc_context context;
    uint64_t* program_begin;
};

struct proc_queue {
    struct proc queue[10];
    int8_t back;
};

struct kernel_state {
    struct proc_queue proc_queue;
    struct physical_page* page_arr;
    size_t page_arr_size;
};

// load_process.s
extern void load_process(struct proc_context* context);
// save_process.s
extern void save_process(struct proc_context* context);
bool queue_is_full(struct proc_queue* queue);
bool queue_is_empty(struct proc_queue* queue);
void shift_queue(struct proc_queue* queue);
void push_queue(struct proc_queue* queue, struct proc proc);
void pop_queue(struct proc_queue* queue);
struct proc proc_create(uint64_t pid, uint64_t x2_value, struct proc_queue* proc_queue, struct copy_info* copy_info, struct physical_page* page_arr, size_t page_arr_size);

#endif
