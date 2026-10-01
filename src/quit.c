

#include "mem.h"
#include "proc.h"
#include "uart.h"
#include <stdint.h>

extern struct kernel_state kernel_state;

void quit_handler() {
    const uint64_t HEX_NUMBER_LEN = 18;
    puts("quitting process ", sizeof("quitting process "));
    uint64_t pid = kernel_state.proc_queue.queue[0].pid;
    char pid_str[HEX_NUMBER_LEN];
    uint64_t_to_string(pid_str, pid);
    puts(pid_str, sizeof(pid_str));
    puts("\n", sizeof("\n"));

    const uint64_t va_offset = 0xffffffff00000000;
    struct proc process_to_quit = kernel_state.proc_queue.queue[0];
    process_to_quit.state = DEAD;
    // remove program from memory
    uint64_t* page = (uint64_t*)((uint64_t)process_to_quit.program_begin - va_offset);

    // remove from scheduling queue
    pop_queue(&kernel_state.proc_queue);
    // page can be used again
    puts("freeing memory\n", sizeof("freeing memory\n"));
    free(page, kernel_state.page_arr);
    if (queue_is_empty(&kernel_state.proc_queue)) {
        puts("kernel done, halting now", sizeof("kernel done, halting now"));
        asm volatile("b halt");
    }
    else {
        // continue executing
        puts("loading process ", sizeof("loading process "));
        uint64_t pid2 = kernel_state.proc_queue.queue[0].pid;
        char pid_str2[HEX_NUMBER_LEN];
        uint64_t_to_string(pid_str2, pid2);
        puts(pid_str2, sizeof(pid_str2));
        puts("\n", sizeof("\n"));
        load_process(&kernel_state.proc_queue.queue[0].context);
    }
}