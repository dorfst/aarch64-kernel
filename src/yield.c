#include "uart.h"
#include <stdint.h>
#include "proc.h"

extern struct kernel_state kernel_state;

void yield_handler() {
    puts("yielding process ", sizeof("yielding process "));
    const uint64_t HEX_NUMBER_LEN = 18;
    uint64_t pid = kernel_state.proc_queue.queue[0].pid;
    char pid_str[HEX_NUMBER_LEN];
    uint64_t_to_string(pid_str, pid);
    puts(pid_str, sizeof(pid_str));
    puts("\n", sizeof("\n"));
    // start running the next process
    kernel_state.proc_queue.queue[0].state = READY;
    shift_queue(&kernel_state.proc_queue);
    kernel_state.proc_queue.queue[0].state = RUNNING;

    puts("loading process ", sizeof("loading process "));
    uint64_t pid2 = kernel_state.proc_queue.queue[0].pid;
    char pid_str2[HEX_NUMBER_LEN];
    uint64_t_to_string(pid_str2, pid2);
    puts(pid_str2, sizeof(pid_str2));
    puts("\n", sizeof("\n"));
    load_process(&kernel_state.proc_queue.queue[0].context);
}
