

#include "mem.h"
#include "proc.h"
#include "uart.h"
#include <stdint.h>

extern struct kernel_state kernel_state;

void quit_handler() {
    puts("quitting process\n", sizeof("quitting process\n"));
    const uint64_t va_offset = 0xffffffff00000000;
    struct proc process_to_quit = kernel_state.proc_queue.queue[0];
    process_to_quit.state = DEAD;
    // remove program from memory
    uint64_t* page = (uint64_t*)((uint64_t)process_to_quit.program_begin - va_offset);
    zero_page(page);

    // remove from scheduling queue
    pop_queue(&kernel_state.proc_queue);
    // page can be used again
    puts("freeing memory\n", sizeof("freeing memory\n"));
    free(page, kernel_state.page_arr);

    // continue executing
    puts("loading next process\n", sizeof("loading next process\n"));
    load_process(&kernel_state.proc_queue.queue[0].context);
}