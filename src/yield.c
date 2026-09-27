#include "uart.h"
#include <stdint.h>
#include "proc.h"

extern struct kernel_state kernel_state;

void yield_handler() {
    puts("yielding\n", sizeof("yielding"));

    // start running the next process
    kernel_state.proc_queue.queue[0].state = READY;
    shift_queue(&kernel_state.proc_queue);
    kernel_state.proc_queue.queue[0].state = RUNNING;
    puts("loading next process\n", sizeof("loading next process\n"));
    load_process(&kernel_state.proc_queue.queue[0].context);
}
