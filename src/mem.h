#ifndef MEM
#define MEM

#include <stdint.h>
#include <stddef.h>

enum PAGE_TYPE {PAGE_S, PAGE_T, KERNEL_FREE, KERNEL_USED, KERNEL_BOOT, USER_FREE, USER_USED};

// size is always 4KB
// implies that only 256 processes can be run at max but the kernel will never run more than that
struct physical_page {
    uint8_t pid;
    uint8_t* start;
    enum PAGE_TYPE type;
};

struct copy_info {
    uint8_t* origin;
    uint8_t* destination;
    uint64_t size;
};

void* frame_alloc(uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size);
void zero_page(uint64_t* page_address);
void* malloc(uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size);
void free(uint64_t* page_address, struct physical_page* page_arr);
struct copy_info cmalloc(uint64_t* start, uint64_t* end, uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size);
void copy(struct copy_info* copy_info);
uint8_t verify_copy(struct copy_info* copy_info);
uint64_t* create_tables(uint8_t pid, struct physical_page* page_arr, size_t page_arr_size, size_t proc_load_size);
void populate_tables(uint64_t* l1, uint64_t* proc_start, size_t proc_load_size);
void *memcpy(void *dest, const void *src, size_t n);


#endif
