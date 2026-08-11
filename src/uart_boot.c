#include "uart.h"

// https://krinkinmu.github.io/2020/11/29/PL011.html
// const virtual_address_offset = 0xffffffff00000000;
const volatile uint32_t* UART_BASE = (uint32_t*) 0x09000000;
const uint32_t DR_OFFSET = 0x000;
const uint32_t FR_OFFSET = 0x018;
const uint32_t IBRD_OFFSET = 0x024;
const uint32_t FBRD_OFFSET = 0x028;
const uint32_t LCR_OFFSET = 0x02c;
const uint32_t CR_OFFSET = 0x030;
const uint32_t IMSC_OFFSET = 0x038;
const uint32_t DMACR_OFFSET = 0x048;


volatile uint32_t* getRegisterBoot(const volatile uint32_t* base, uint32_t offset) {
    return (volatile uint32_t*)((void*)base + offset);
}

void waitForTXBoot() {
    while((*getRegisterBoot(UART_BASE, FR_OFFSET) & (1 << 3)) != 0) {}
}

// enable uart, enable tx, disable dma controller, set baud rate
int setupBootUART() {
    uint32_t cr = *getRegisterBoot(UART_BASE, CR_OFFSET);
    uint32_t lcr = *getRegisterBoot(UART_BASE, LCR_OFFSET);
    uint32_t ibrd, fbrd;

    *getRegisterBoot(UART_BASE, CR_OFFSET) = cr | 0x0;

    waitForTXBoot();

    *getRegisterBoot(UART_BASE, LCR_OFFSET) = (lcr & ~(1 << 4));

    const uint32_t div = 4 * 24000000 / 115200;

    fbrd = div & 0x3f;
    ibrd = (div >> 6) & 0xffff;
    *getRegisterBoot(UART_BASE, IBRD_OFFSET) = ibrd;
    *getRegisterBoot(UART_BASE, FBRD_OFFSET) = fbrd;

    lcr = 0x0;
    lcr |= (7 & 0x3) << 5;

    *getRegisterBoot(UART_BASE, IMSC_OFFSET) = 0x7ff;
    *getRegisterBoot(UART_BASE, DMACR_OFFSET) = 0x0;
    *getRegisterBoot(UART_BASE, CR_OFFSET) = (1 << 8);
    *getRegisterBoot(UART_BASE, CR_OFFSET) = (1 << 8) | 1;

    return 0;
}

int putsBoot(char* data, size_t size) {
    waitForTXBoot();

    for (size_t i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            *getRegisterBoot(UART_BASE, DR_OFFSET) = '\r';
            waitForTXBoot();
        }
        *getRegisterBoot(UART_BASE, DR_OFFSET) = data[i];
        waitForTXBoot();
    }

    return 0;

}