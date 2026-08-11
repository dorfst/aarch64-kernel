#include <stdint.h>
#include <stddef.h>
#include "uart.h"

enum PAGE_TYPE {PAGE_S, PAGE_T, KERNEL_FREE, KERNEL_USED, KERNEL_BOOT, USER_FREE, USER_USED};

// size is always 4KB
// implies that only 256 processes can be run at max but the kernel will never run more than that
struct physical_page {
    uint8_t pid;
    uint8_t* start;
    enum PAGE_TYPE type;
};

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

void* frame_alloc(uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size) {
    if (type == KERNEL_FREE || type == USER_FREE) {
        return NULL;
    }

    // number of elements, as opposed to size in bytes
    page_arr_size = (size_t)(page_arr_size / sizeof(struct physical_page));

    // find next free page
    uint64_t counter = 0;
    while (counter < page_arr_size) {
        if (page_arr[counter].type == KERNEL_FREE || page_arr[counter].type == USER_FREE) {
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

// if something goes wrong, check if this is right at all
void create_missing_table_descriptor_entry(struct physical_page* page_arr, size_t page_arr_size, uint64_t* current_table, uint64_t expected_index) {
    uint64_t va_offset = 0xffffffff00000000;
    uint64_t table_descriptor_flag = 0x0000000000000003;
    uint64_t output_address_table_decriptor_mask = 0x0000FFFFFFFFF000;

    uint64_t* new_table = (uint64_t*)frame_alloc(0, PAGE_T, page_arr, page_arr_size);

    current_table[expected_index] = (((uint64_t)new_table - va_offset) & output_address_table_decriptor_mask) | table_descriptor_flag;

    zero_page(new_table);
}

// find where to go place a page table entry, gives exact address to place it in
uintptr_t walk_page_tables(struct physical_page* page_arr, size_t page_arr_size, uint64_t* top_level_table, uintptr_t page_address_to_alloc) {
    uint64_t va_offset = 0xffffffff00000000;
    uint64_t output_address_table_decriptor_mask = 0x0000FFFFFFFFF000;
    // uint64_t output_address_block_descriptor_mask = 0x0000FFFFC0000000;
    uint64_t table_page_descriptor_mask = 0x0000000000000003;
    uint64_t block_descriptor_mask = 0x1;

    uint64_t* current_table = top_level_table;
    for (uint8_t i = 1; i < 4; ++i) {
        uint8_t shift_amount = 12 + 9 * (3 - i);
        uint64_t index = ((uint64_t)page_address_to_alloc >> shift_amount) & 0x1FF;
        uint64_t* current_entry = (uint64_t*)((uint64_t)current_table + (8 * index));
        uint64_t* next_table = 0;
        if (*current_entry != 0) {
            if ((*current_entry & table_page_descriptor_mask) == 3 && i != 3) next_table = (uint64_t*)((*current_entry & output_address_table_decriptor_mask) + va_offset);
        }
        else if (*current_entry == 0 && i != 3) {
            // the level where the address given does not point to a valid entry (i.e. a table descriptor is missing)
            create_missing_table_descriptor_entry(page_arr, page_arr_size, current_table, index);
        }
        else if ((*current_entry & block_descriptor_mask) == 1 && i != 3) return 4; // 4 is a block descriptor, 5 means "already populated", 6 means something went wrong
        else if ((*current_entry & table_page_descriptor_mask) == 3 && i == 3) return 5;
        else if (*current_entry == 0 && i == 3) {
            return (uintptr_t)current_entry;
        }
        
        current_table = next_table != 0 ? (uint64_t*)next_table : (uint64_t*)current_table;

    }

    return 6;


}

void make_new_l3_entry(uint64_t* entry_address, uint64_t entry_value) {
    *entry_address = entry_value;

    asm volatile("dsb ish" ::: "memory");
}

void* malloc(uint8_t pid, uint64_t* top_level_table, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size) {
    uint64_t va_offset = 0xffffffff00000000;
    uint64_t output_address_page_descriptor_mask = 0x0000FFFFFFFFF000;
    uint64_t kernel_mem_attribs = 0x0040000000000403;
    void* page = frame_alloc(pid, type, page_arr, page_arr_size);

    uint64_t* page_table_entry_address = (uint64_t*)walk_page_tables(page_arr, page_arr_size, top_level_table, (uintptr_t)page);

    uint64_t page_descriptor = (((uint64_t)(page) & output_address_page_descriptor_mask) | kernel_mem_attribs);

    make_new_l3_entry(page_table_entry_address, page_descriptor);

    zero_page((uint64_t*)page);

    return (void*)(page + va_offset);

}

int main() {
    uint64_t va_offset = 0xffffffff00000000;
    uint64_t* l1_top_level_table = (uint64_t*)(read_x8() + va_offset);
    struct physical_page* page_arr = (struct physical_page*)(read_x9() + va_offset);
    size_t page_arr_size = (size_t)read_x10();

    uint64_t* page = malloc(0, l1_top_level_table, KERNEL_USED, page_arr, page_arr_size);

    page[0] = 0xE110D00D;

    setupUART();
    puts("memory write success!!!", sizeof("memory write success!!!"));

    return 0;
}



