#include "uart.h"
#include <stdint.h>

// https://krinkinmu.github.io/2020/11/29/PL011.html
const uint64_t virtual_address_offset = 0xffffffff00000000;
const volatile uint32_t* UART_BASE = (uint32_t*)(0x09000000 + virtual_address_offset);
const uint32_t DR_OFFSET = 0x000;
const uint32_t FR_OFFSET = 0x018;
const uint32_t IBRD_OFFSET = 0x024;
const uint32_t FBRD_OFFSET = 0x028;
const uint32_t LCR_OFFSET = 0x02c;
const uint32_t CR_OFFSET = 0x030;
const uint32_t IMSC_OFFSET = 0x038;
const uint32_t DMACR_OFFSET = 0x048;


volatile uint32_t* getRegister(const volatile uint32_t* base, uint32_t offset) {
    return (volatile uint32_t*)((void*)base + offset);
}

void waitForTX() {
    while((*getRegister(UART_BASE, FR_OFFSET) & (1 << 3)) != 0) {}
}

// enable uart, enable tx, disable dma controller, set baud rate
int setupUART() {
    uint32_t cr = *getRegister(UART_BASE, CR_OFFSET);
    uint32_t lcr = *getRegister(UART_BASE, LCR_OFFSET);
    uint32_t ibrd, fbrd;

    *getRegister(UART_BASE, CR_OFFSET) = cr | 0x0;

    waitForTX();

    *getRegister(UART_BASE, LCR_OFFSET) = (lcr & ~(1 << 4));

    const uint32_t div = 4 * 24000000 / 115200;

    fbrd = div & 0x3f;
    ibrd = (div >> 6) & 0xffff;
    *getRegister(UART_BASE, IBRD_OFFSET) = ibrd;
    *getRegister(UART_BASE, FBRD_OFFSET) = fbrd;

    lcr = 0x0;
    lcr |= (7 & 0x3) << 5;

    *getRegister(UART_BASE, IMSC_OFFSET) = 0x7ff;
    *getRegister(UART_BASE, DMACR_OFFSET) = 0x0;
    *getRegister(UART_BASE, CR_OFFSET) = (1 << 8);
    *getRegister(UART_BASE, CR_OFFSET) = (1 << 8) | 1;

    return 0;
}

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

int puts(const char* data, size_t size) {
    waitForTX();

    for (size_t i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            *getRegister(UART_BASE, DR_OFFSET) = '\r';
            waitForTX();
        }
        *getRegister(UART_BASE, DR_OFFSET) = data[i];
        waitForTX();
    }

    return 0;

}