#include <stdint.h>
#include "uart_boot.h"


// nibble is 4 bits but there is no 4-bit type
char nibble_to_hex(uint8_t nibble) {
    if (nibble == 0) return '0';
    else if (nibble == 1) return '1';
    else if (nibble == 2) return '2';
    else if (nibble == 3) return '3';
    else if (nibble == 4) return '4';
    else if (nibble == 5) return '5';
    else if (nibble == 6) return '6';
    else if (nibble == 7) return '7';
    else if (nibble == 8) return '8';
    else if (nibble == 9) return '9';
    else if (nibble == 10) return 'A';
    else if (nibble == 11) return 'B';
    else if (nibble == 12) return 'C';
    else if (nibble == 13) return 'D';
    else if (nibble == 14) return 'E';
    else if (nibble == 15) return 'F';

    return '\0';
}

void uint64_t_to_string(char* str, uint64_t val) {
    // each digit needs its ASCII equivalent
    /*
    * how to isolate digits? read ptr directly. 
    * little-endian, so first byte is the last digit
    */
    uint8_t* val_ptr = (uint8_t*)&val;

    uint64_t array_counter = 17;
    str[0] = '0';
    str[1] = 'x';

    for (uint64_t i = 0; i < 8; ++i) {
        uint8_t high_bits = (val_ptr[i] & 0xf0) >> 4;
        uint8_t low_bits = (val_ptr[i] & 0xf);

        char high_bits_char = nibble_to_hex(high_bits);
        char low_bits_char = nibble_to_hex(low_bits);

        str[array_counter] = low_bits_char;
        str[array_counter - 1] = high_bits_char;

        array_counter -= 2;

    }
}

void print_error(uint64_t esr_el1, uint64_t far_el1, uint64_t elr_el1) {
    // first two characters "0x", 16 for 64-bit number
    const uint64_t HEX_NUMBER_LEN = 18;
    setupBootUART();
    char esr_el1_str[HEX_NUMBER_LEN];
    char far_el1_str[HEX_NUMBER_LEN];
    char elr_el1_str[HEX_NUMBER_LEN];

    uint64_t_to_string(esr_el1_str, esr_el1);
    uint64_t_to_string(far_el1_str, far_el1);
    uint64_t_to_string(elr_el1_str, elr_el1);


    putsBoot("ESR_EL1: ", sizeof("ESR_EL1: "));
    putsBoot(esr_el1_str, sizeof(esr_el1_str));
    putsBoot("\n", sizeof("\n"));
    putsBoot("FAR_EL1: ", sizeof("FAR_EL1: "));
    putsBoot(far_el1_str, sizeof(far_el1_str));
    putsBoot("\n", sizeof("\n"));
    putsBoot("ELR_EL1: ", sizeof("ELR_EL1: "));
    putsBoot(elr_el1_str, sizeof(elr_el1_str));
    putsBoot("\n", sizeof("\n"));

    char test[HEX_NUMBER_LEN];
    uint64_t_to_string(test, 256);
    putsBoot(test, sizeof(test));
}