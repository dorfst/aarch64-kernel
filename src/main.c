#include "uart.h"
#include "mem.h"
#include "proc.h"
#include <stdint.h>
#include <stddef.h>

// linker symbols for the dummy program
extern uint64_t _dummy_start;
uint64_t* dummy_start = &_dummy_start;

extern uint64_t _dummy_end;
uint64_t* dummy_end = &_dummy_end;

extern uint64_t _dummy_text_start;
uint64_t* dummy_text_start = &_dummy_text_start;

extern uint64_t _dummy_text_end;
uint64_t* dummy_text_end = &_dummy_text_end;

uint64_t read_x8() {
    uint64_t val;
    asm volatile("mov %0, x8" : "=r"(val));
    return val;
}

uint64_t read_x9() {
    uint64_t val;
    asm volatile("mov %0, x9" : "=r"(val));
    return val;
}
uint64_t read_x10() {
    uint64_t val;
    asm volatile("mov %0, x10" : "=r"(val));
    return val;
}

void yield() {
    asm volatile("mov x8, #20\n\t"
                 "svc #0"
                 ::: "x8", "memory");
}


struct kernel_state kernel_state;
struct proc_queue queue;

int main() {
    const uint64_t va_offset = 0xffffffff00000000;
    struct physical_page* page_arr = (struct physical_page*)(read_x9() + va_offset);
    size_t page_arr_size = (size_t)read_x10();

    queue.back = -1;

    kernel_state.proc_queue = queue;
    kernel_state.page_arr = page_arr;
    kernel_state.page_arr_size = page_arr_size;

    struct copy_info copy_info_1 = cmalloc(dummy_start, dummy_end, 1, USER_USED, page_arr, page_arr_size);
    copy(&copy_info_1);

    uint8_t result = verify_copy(&copy_info_1);

    setupUART();
    if (result == 0) puts("memory copy success!!!", sizeof("memory copy success!!!"));


    struct proc process = proc_create(1, 10, &kernel_state.proc_queue, &copy_info_1, kernel_state.page_arr, kernel_state.page_arr_size);

    struct copy_info copy_info_2 = cmalloc(dummy_start, dummy_end, 2, USER_USED, page_arr, page_arr_size);
    copy(&copy_info_2);
    proc_create(2, 20, &kernel_state.proc_queue, &copy_info_2, kernel_state.page_arr, kernel_state.page_arr_size);

    process.state = RUNNING;
    load_process(&process.context);


    return 0;
}



