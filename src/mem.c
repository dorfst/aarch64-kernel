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

void free(uint64_t* page_address, struct physical_page* page_arr) {
    uint64_t index = (uint64_t)page_address / 0x1000;
    page_arr[index].type = (uint64_t)page_address < 0x440000000 ? KERNEL_FREE : USER_FREE;
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

uint64_t* create_tables(uint8_t pid, struct physical_page* page_arr, size_t page_arr_size, size_t proc_load_size) {
    // not PAGE_T since that is reserved for kernel's page tables
    uint64_t* l1 = malloc(pid, KERNEL_USED, page_arr, page_arr_size);

    uint64_t va_offset = 0xffffffff00000000;
    uint64_t output_address_table_descriptor_mask = 0x0000FFFFFFFFF000;
    uint64_t table_descriptor_mask = 0x0000000000000003;

    uint64_t two_mib = 0x200000;
    uint64_t four_kib = 0x1000;
    uint64_t l2_tables_needed = (proc_load_size + two_mib - 1) / two_mib;
    uint64_t l3_tables_needed = (proc_load_size + four_kib - 1) / four_kib;

    for (uint64_t i = 0; i < l2_tables_needed; ++i) {
        uint64_t* l2 = malloc(pid, KERNEL_USED, page_arr, page_arr_size);
        l1[i] = (((uint64_t)l2 - va_offset) & output_address_table_descriptor_mask) | table_descriptor_mask;
    }

    for (uint64_t i = 0; i < l3_tables_needed; ++i) {
        uint64_t* current_l2 = (uint64_t*)((l1[i] & output_address_table_descriptor_mask) + va_offset);
        for (int j = 0; j < 512; ++j) {
            uint64_t* l3 = malloc(pid, KERNEL_USED, page_arr, page_arr_size);
            current_l2[j] = (((uint64_t)l3 - va_offset) & output_address_table_descriptor_mask) | table_descriptor_mask;
        }
    }

    return l1;

}


void populate_tables(uint64_t* l1, uint64_t* proc_start, size_t proc_load_size) {
    uint64_t output_address_table_descriptor_mask = 0x0000FFFFFFFFF000;
    uint64_t output_address_page_descriptor_mask = 0x0000FFFFFFFFF000;
    // 0000000000000000000000000000000000000000000000000000110001000011
    uint64_t regular_mem_attribs = 0x0000000000000C43;
    // uint64_t table_descriptor_mask = 0x0000000000000003;
    uint64_t va_offset = 0xffffffff00000000;

    uint64_t l1_counter = 0;
    uint64_t l2_counter = 0;
    // uint64_t l3_counter = 0;

    uint64_t current = (uint64_t)proc_start;

    while (l1[l1_counter] != 0) {
        uint64_t l2_entry = l1[l1_counter];
        uint64_t* l2 = (uint64_t*)(((uint64_t)(l2_entry) & output_address_table_descriptor_mask) + va_offset);
        while (l2[l2_counter] != 0 && current < (uint64_t)proc_start + proc_load_size) {
            uint64_t l3_entry = l2[l2_counter];
            uint64_t* l3 = (uint64_t*)(((uint64_t)(l3_entry) & output_address_table_descriptor_mask) + va_offset);
            for (uint64_t i = 0; i < 512; ++i) {
                l3[i] = ((current - va_offset) & output_address_page_descriptor_mask) | regular_mem_attribs;
                current += 0x1000;
                if (current > (uint64_t)proc_start + proc_load_size) {
                    break;
                }
            }

            ++l2_counter;
        }
        ++l1_counter;
    }

}

// for copying structs by value
void* memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}