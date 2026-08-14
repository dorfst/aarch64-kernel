#include "mem.h"
#include <stdint.h>

void* frame_alloc(uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size) {
    if (type == KERNEL_FREE || type == USER_FREE) {
        return NULL;
    }

    // number of elements, as opposed to size in bytes
    page_arr_size = (size_t)(page_arr_size / sizeof(struct physical_page));

    // find next free page
    uint64_t counter = 0;
    while (counter < page_arr_size) {
        if ((page_arr[counter].type == KERNEL_FREE && (type == KERNEL_USED ||
            type == PAGE_S || type == PAGE_T)) || (page_arr[counter].type == USER_FREE
            && type == USER_USED)) {
            page_arr[counter].type = type;
            page_arr[counter].pid = pid;

            return (void*)(page_arr[counter].start);
        }
        ++counter;
    }

    // no pages free at all
    return NULL;

}

void zero_page(uint64_t* page_address) {
    uint64_t va_offset = 0xffffffff00000000;
    uint64_t* base = (uint64_t*)((uint64_t)page_address + va_offset);
        for (uint64_t i = 0; i < 512; ++i) {
            base[i] = (uint64_t)0;
        }
}

void* malloc(uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size) {
    uint64_t va_offset = 0xffffffff00000000;
    void* page = frame_alloc(pid, type, page_arr, page_arr_size);

    zero_page((uint64_t*)page);

    return (void*)(page + va_offset);

}

// special malloc for copying data from one place to another
struct copy_info cmalloc(uint64_t* start, uint64_t* end, uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size) {
    uint64_t size = (uint64_t)((uint64_t)end - (uint64_t)start);

    uint64_t num_pages_to_alloc = (size + 4096 - 1) / 4096;


    uint8_t* page = (uint8_t*)malloc(pid, type, page_arr, page_arr_size);

    if (num_pages_to_alloc > 1) {
        malloc(0, type, page_arr, page_arr_size);
    }

    struct copy_info copy_info = {(uint8_t*)start, page, size};

    return copy_info;
}

void copy(struct copy_info* copy_info) {
    for (uint64_t i = 0; i < copy_info->size; ++i) {
        copy_info->destination[i] = copy_info->origin[i];
    }
}

uint8_t verify_copy(struct copy_info* copy_info) {
    for (uint64_t i = 0; i < copy_info->size; ++i) {
        if (copy_info->destination[i] != copy_info->origin[i]) {
            return 1;
        }
    }
    return 0;
}