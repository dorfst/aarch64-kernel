#include "uart.h"
#include "mem.h"
#include "proc.h"
#include <stdint.h>
#include <stddef.h>

struct kernel_state {
    struct proc proc_list[256];
    struct proc proc_queue[256];
    struct physical_page* page_arr;
    size_t page_arr_size;
};

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



extern void load_process(struct proc_context* context);

int main() {
    const uint64_t va_offset = 0xffffffff00000000;
    // uint64_t* l1_top_level_table = (uint64_t*)(read_x8() + va_offset);
    struct physical_page* page_arr = (struct physical_page*)(read_x9() + va_offset);
    size_t page_arr_size = (size_t)read_x10();

    struct copy_info copy_info = cmalloc(dummy_start, dummy_end, 1, USER_USED, page_arr, page_arr_size);
    copy(&copy_info);

    uint8_t result = verify_copy(&copy_info);

    setupUART();
    if (result == 0) puts("memory copy success!!!", sizeof("memory copy success!!!"));

    // uint64_t text_start_offset = (uint64_t)dummy_text_start - (uint64_t)dummy_start;

    uint64_t* l1 = create_tables(1, page_arr, page_arr_size, (size_t)copy_info.size);
    populate_tables(l1, (uint64_t*)copy_info.destination, copy_info.size);

    struct proc test_process = {1, READY, 0, NULL, (uint64_t*)copy_info.destination};

    struct proc_context test_process_context;
    test_process_context.l1_table = l1;
    for (int i = 0; i < 30; ++i) {
        if (i != 2) {
            test_process_context.general_purpose[i] = 0;
        }
        else {
            test_process_context.general_purpose[i] = 10;
        }
    }
    test_process_context.l1_table = (uint64_t*)((uint64_t)l1 - va_offset);
    test_process_context.pc = 0;
    test_process_context.sp = 0;
    test_process_context.spsr = 0;

    test_process.context = &test_process_context;

    



    load_process(&test_process_context);


    return 0;
}



