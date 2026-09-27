#include <stdint.h>
#include <stddef.h>

// set up linker symbols
extern uint64_t _OFFSET;
uint64_t* va_offset = &_OFFSET;

extern uint64_t _KERNEL_PA_BEGIN;
uint64_t* kernel_pa_begin = &_KERNEL_PA_BEGIN;

extern uint64_t _KERNEL_PA_END;
uint64_t* kernel_pa_end = &_KERNEL_PA_END;

extern uint64_t _KERNEL_VA_BEGIN;
uint64_t* kernel_va_begin = &_KERNEL_VA_BEGIN;

extern uint64_t _KERNEL_VA_END;
uint64_t* kernel_va_end = &_KERNEL_VA_END;

extern uint64_t _exception_vector_address;
uint64_t* exception_vector_address = &_exception_vector_address;

const uint64_t MAX_KERN_ADDRESS = 0x40000000 + 0x4000000;

// this is where our free space begins, aligned to 4KB
extern uint64_t _end_pa;
uint64_t* end_pa = &_end_pa;

// page struct, page table
enum PAGE_TYPE {PAGE_S, PAGE_T, KERNEL_FREE, KERNEL_USED, KERNEL_BOOT, USER_FREE, USER_USED};


// size is always 4KB
// implies that only 256 processes can be run at max but the kernel will never run more than that
struct physical_page {
    uint8_t pid;
    uint8_t* start;
    enum PAGE_TYPE type;
};

// map out all of RAM into pages
uint64_t setup_page_structs(struct physical_page* page_arr) {
    // initialised to beginning of RAM
    uint64_t current = 0x40000000;
    uint64_t counter = 0;

    // we need to account for total kernel memory usage after setting up memory too
    // number of page structs needed to cover 128MB
    size_t size_of_arr = (0x8000000 / 0x1000);

    // initialise to free by default, in mark_used_kernel_areas we set make sure that currently used memory is marked correctly
    for(uint64_t i = 0; i < size_of_arr; ++i) {
        enum PAGE_TYPE free_type = current <= 0x40000000 + 0x4000000 - 1 ? KERNEL_FREE : USER_FREE;
        page_arr[i] = (struct physical_page){0, (uint8_t*)current, free_type};
        current += 0x1000;
        ++counter;
    }
    
    // size of page list
    return sizeof(struct physical_page) * counter;
}

void mark_used_kernel_areas(struct physical_page* page_arr, size_t page_arr_size) {
    // 0x4000_0000 to kernel_pa_begin - 1: KERNEL_USED
    // kernel_pa_begin to kernel_pa_end: KERNEL_BOOT
    // kernel_va_begin - offset to kernel_va_end - offset: KERNEL_USED
    // _end_pa to _end_pa + page_arr_size: PAGE_S
    uint64_t kernel_used_range_1 = (uint64_t)(0x8000 + 0x1000 - 1) / 0x1000;
    uint64_t kernel_boot_range = (((uint64_t)((uint64_t)kernel_pa_end - (uint64_t)kernel_pa_begin) + 0x1000 - 1) / 0x1000);
    uint64_t kernel_used_range_2 = (uint64_t)(((uint64_t)end_pa - (uint64_t)((uint64_t)kernel_va_end - (uint64_t)va_offset) + 0x1000 - 1) / 0x1000);
    uint64_t page_s_range = (uint64_t)((uint64_t)((uint64_t)page_arr_size + 0x1000 - 1) / 0x1000);

    uint64_t current_pos = 0;

    for (uint64_t i = current_pos; i < kernel_used_range_1; ++i) {
        page_arr[i].type = KERNEL_USED;
    }

    current_pos += kernel_used_range_1;

    for (uint64_t i = current_pos; i < current_pos + kernel_boot_range; ++i) {
        page_arr[i].type = KERNEL_BOOT;
    }

    current_pos += kernel_boot_range;    

    for (uint64_t i = current_pos; i < current_pos + kernel_used_range_2; ++i) {
        page_arr[i].type = KERNEL_USED;
    }

    current_pos += kernel_used_range_2;

    for (uint64_t i = current_pos; i < current_pos + page_s_range; ++i) {
        page_arr[i].type = PAGE_S;
    }

}


void* page_alloc(uint8_t pid, enum PAGE_TYPE type, struct physical_page* page_arr, size_t page_arr_size) {
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

            // set the whole page to zero (for some purposes, e.g. page tables, this makes walking the tables much easier)
            uint64_t* base = (uint64_t*)page_arr[counter].start;
            for (uint64_t i = 0; i < 512; ++i) {
                base[i] = (uint64_t)0;
            }

            return (void*)page_arr[counter].start;
            break;
        }
        ++counter;
    }

    // no pages free at all
    return NULL;

}


void kernel_direct_map(uint64_t* l2_table, struct physical_page* page_arr, size_t page_arr_size) {
    uint64_t output_address_table_descriptor_mask = 0x0000FFFFFFFFF000;
    uint64_t output_address_page_descriptor_mask = 0x0000FFFFFFFFF000;
    uint64_t kernel_mem_attribs = 0x0040000000000403;
    uint64_t table_descriptor_mask = 0x0000000000000003;

    uint64_t current = 0x40000000;

    for (uint64_t i = 0; i < 64; ++i) {
        uint64_t* l3_table = (uint64_t*)page_alloc(0, PAGE_T, page_arr, page_arr_size);
        l2_table[i] = ((uint64_t)l3_table & output_address_table_descriptor_mask) | table_descriptor_mask;


        for (uint64_t j = 0; j < 512; ++j) {
            l3_table[j] = (current & output_address_page_descriptor_mask) | kernel_mem_attribs;
            current += 0x1000;
        }
    }
}


static inline void write_tcr_el1(uint64_t val) {
    asm volatile("msr tcr_el1, %0" :: "r"(val) : "memory");
}

static inline void write_mair_el1(uint64_t val) {
    asm volatile("msr mair_el1, %0" :: "r"(val) : "memory");
}

static inline void write_ttbr1_el1(uint64_t val) {
    asm volatile("msr ttbr1_el1, %0" :: "r"(val) : "memory");
}

static inline void write_ttbr0_el1(uint64_t val) {
    asm volatile("msr ttbr0_el1, %0" :: "r"(val) : "memory");
}

static inline void write_sctlr_el1(uint64_t val) {
    asm volatile("msr sctlr_el1, %0\n\t"
                 "isb" 
                 :: "r"(val) : "memory");
}

static inline uint64_t read_sctlr_el1() {
    uint64_t val;
    asm volatile("mrs %0, sctlr_el1\n\t" : "=r"(val));
    return val;
}

static inline void write_vbar_el1(uint64_t val) {
    asm volatile("msr vbar_el1, %0" :: "r"(val) : "memory");
}


void boot() {
    // set exception vector
    write_vbar_el1((uint64_t)exception_vector_address);

    // initialise page array
    struct physical_page* page_arr = (struct physical_page*)end_pa;
    uint64_t page_arr_size = setup_page_structs(page_arr);

    // setup 0x4000_0000 up until end_pa - 1 as KERNEL, then from end_pa to end as PAGE_S
    mark_used_kernel_areas(page_arr, page_arr_size);

    /* pages setup, current RAM usage is mapped, now set up page tables for the CPU
    * if something goes wrong, consider the 1 GB block descriptor
    * 3 levels
    * L1: 1 table: 1 1GB block descriptor, 1 1GB table descriptor (one for MMIO, one for RAM)
    * L2: 1 table for RAM (variable number of entries depending on allocations)
    * L3: 64 tables at a maximum, but at least two (one for boot code, another for rest of kernel). (assuming 128MB RAM)
    * 
    */

    uint64_t output_address_table_descriptor_mask = 0x0000FFFFFFFFF000;
    uint64_t output_address_block_descriptor_mask = 0x0000FFFFC0000000;
    uint64_t output_address_page_descriptor_mask = 0x0000FFFFFFFFF000;

    uint64_t table_descriptor_mask = 0x0000000000000003;
    // uint64_t valid_entry_mask = 0x0000000000000001;

    // setup mmio block entry

    uint64_t mmio_attribs = 0x0060000000000405;

    uint64_t mmio_1gb_block_entry = (0x0000000000000000 & output_address_block_descriptor_mask) | mmio_attribs;


    // initialise tables
    uint64_t* l1_table_kernel_boot = (uint64_t*)page_alloc(0, PAGE_T, page_arr, page_arr_size);
    uint64_t* l2_table_kernel_boot = (uint64_t*)page_alloc(0, PAGE_T, page_arr, page_arr_size);
    uint64_t* l3_table_kernel_boot = (uint64_t*)page_alloc(0, PAGE_T, page_arr, page_arr_size);
    uint64_t* l1_table_kernel = (uint64_t*)page_alloc(0, PAGE_T, page_arr, page_arr_size);
    uint64_t* l2_table_kernel_ram = (uint64_t*)page_alloc(0, PAGE_T, page_arr, page_arr_size);

    l1_table_kernel[0] = mmio_1gb_block_entry;

    // set up l1 -> l2 -> l3 links for boot code page tables
    l1_table_kernel_boot[1] = ((uint64_t)l2_table_kernel_boot & output_address_table_descriptor_mask) | table_descriptor_mask;
    l2_table_kernel_boot[0] = ((uint64_t)l3_table_kernel_boot & output_address_table_descriptor_mask) | table_descriptor_mask;


    // set up table descriptor for 1GB RAM

    uint64_t kernel_ram_1gb_table_descriptor = (uint64_t)l2_table_kernel_ram;

    l1_table_kernel[1] = (kernel_ram_1gb_table_descriptor & output_address_table_descriptor_mask) | table_descriptor_mask;


    // set up page table for boot code, make sure to load this pointer to this pointer in TTBR0
    uint64_t address_diff = (uint64_t)kernel_pa_end - (uint64_t)kernel_pa_begin;
    uint64_t number_of_pages = (uint64_t)((address_diff + 4096 -1) / 4096);

    uint64_t kernel_mem_attribs = 0x0040000000000403;

    uint64_t current = (uint64_t)kernel_pa_begin;

    for (uint64_t i = 0; i < number_of_pages; ++i) {
        uint64_t entry = (current & output_address_page_descriptor_mask) | kernel_mem_attribs;
        // kernel is loaded at 0x4000_8000, which is 4KB * 8 past 0x4000_0000, hence the i + 8
        l3_table_kernel_boot[i + 8] = entry;
        current += 0x1000;
    }

    // direct map all of physical RAM.
    kernel_direct_map(l2_table_kernel_ram, page_arr, page_arr_size);
    
    // set up registers required for virtual memory

    // TCR_EL1
    const uint64_t tcr_el1_val = 0b0000000000000000000001100000000010110101011000000011010100100000;
    write_tcr_el1(tcr_el1_val);

    // MAIR_EL1
    const uint64_t mair_el1_val = 0b0000000000000000000000000000000000000000000000000000000011111111;
    write_mair_el1(mair_el1_val);

    // TTBR0_EL1
    uint64_t ttbr0_address = (uint64_t)l1_table_kernel_boot;
    write_ttbr0_el1(ttbr0_address);

    // TTBR1_EL1
    uint64_t ttbr1_address = (uint64_t)l1_table_kernel;
    write_ttbr1_el1(ttbr1_address);

    // SCTLR_EL1 (enable mmu!)
    uint64_t enable_mmu = read_sctlr_el1() | 0x1;
    write_sctlr_el1(enable_mmu);

    // pass some stuff through these registers so that the main function has them
    asm volatile("mov x8, %0" :: "r"(l1_table_kernel) : "memory");
    asm volatile("mov x9, %0" :: "r"(page_arr) : "memory");
    asm volatile("mov x10, %0" :: "r"(page_arr_size) : "memory");
}